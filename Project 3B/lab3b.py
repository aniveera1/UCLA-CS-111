#! /usr/bin/env python

from __future__ import print_function
import argparse
import csv
import sys
import os

exit_value = 0

class block_tracker:
	def __init__(self, csv_file):
		"""Collect all data needed for consistency audit"""
		self.num_blocks = 0
		self.inodes = list()
		self.indirect = list()
		self.free_blocks = set()
		self.allocated_blocks = set()
		self.duplicate_blocks = dict()

		# Use group line to determine number of blocks
		with open(csv_file) as opened_csv:
			entry_lines = opened_csv.read().splitlines()

			group_line = entry_lines[1].split(',')
			self.num_blocks = int(group_line[2]) # Max block number

			for line in entry_lines:
				if 'INODE' in line:
					inode_line = line.split(',')
					if inode_line[0] == 'INODE':
						self.inodes.append(inode_line)
						for index in list(range(12,27)):
							self.allocated_blocks.add(inode_line[index])

							if int(inode_line[index]) == 0:
								continue

							if not inode_line[index] in self.duplicate_blocks:
								self.duplicate_blocks[inode_line[index]] = list()
							if index == 26:
								self.duplicate_blocks[inode_line[index]].append((inode_line[1],65804))
							elif index == 25:
								self.duplicate_blocks[inode_line[index]].append((inode_line[1],268))
							elif index == 24:
								self.duplicate_blocks[inode_line[index]].append((inode_line[1],12))
							else:
								self.duplicate_blocks[inode_line[index]].append((inode_line[1],0))
				elif 'BFREE' in line:
					block_line = line.split(',')
					if block_line[0] == "BFREE":
						self.free_blocks.add(block_line[1])
				elif 'INDIRECT' in line:
					indirect_line = line.split(',')
					if indirect_line[0] == 'INDIRECT':
						self.indirect.append(indirect_line)
						self.allocated_blocks.add(indirect_line[5])
						if not indirect_line[5] in self.duplicate_blocks:
							self.duplicate_blocks[indirect_line[5]] = list()
						self.duplicate_blocks[indirect_line[5]].append((indirect_line[1],indirect_line[3]))

	def check_block_validity(self):
		"""Check that each block pointer points to a valid and unreserved block"""
		for inode in self.inodes:
			for block_addr in range(12,27): # Blocks are in last 15 fields
				if int(inode[block_addr]) > self.num_blocks or int(inode[block_addr]) < 8:
					if int(inode[block_addr]) == 0:
						continue

					error = "INVALID "
					if int(inode[block_addr]) in list(range(1,8)):
						error = "RESERVED "
					indirection_level = ""
					offset = "0"

					if block_addr == 26:
						indirection_level = "TRIPLE INDIRECT "
						offset = "65804"
					elif block_addr == 25:
						indirection_level = "DOUBLE INDIRECT "
						offset = "268"
					elif block_addr == 24:
						indirection_level = "INDIRECT "
						offset = "12"

					print(error + indirection_level + "BLOCK " + str(inode[block_addr]), end='')
					print(" IN INODE " + str(inode[1]), end='')
					print(" AT OFFSET " + offset)
					exit_value = 2

	def check_allocation(self):
		"""Check that every legal block is either allocated or on free block list"""
		for block in self.allocated_blocks:
			if block in self.free_blocks:
				print("ALLOCATED BLOCK " + str(block) + " ON FREELIST")
				exit_value = 2

		all_blocks = map(str, list(range(8, self.num_blocks)))
		for block in all_blocks:
			if not (block in self.free_blocks or block in self.allocated_blocks):
				print("UNREFERENCED BLOCK " + str(block))
				exit_value = 2

	def check_duplicate_blocks(self):
		"""Check that a legal block is only referenced by one file"""
		for key, value in self.duplicate_blocks.items():
			if len(value) > 1:
				value.sort(key=lambda tup: tup[1]) # Sort by offset
				for dupl in value:
					blocktype = ""
					if int(dupl[1]) >= 65804:
						blocktype = "TRIPLE INDIRECT "
					elif int(dupl[1]) >= 268:
						blocktype = "DOUBLE INDIRECT "
					elif int(dupl[1]) >= 12:
						blocktype = "INDIRECT "
					print("DUPLICATE " + blocktype + "BLOCK " + key, end='')
					print(" IN INODE", dupl[0], end='')
					print(" AT OFFSET", dupl[1])
					exit_value = 2

	def audit_consistency(self):
		"""Report to stdout all inconsistencies"""
		self.check_block_validity()
		self.check_allocation()
		self.check_duplicate_blocks()


class inode_tracker:
	def __init__(self, csv_file):
		"""Collect all data needed for consistency audit"""
		self.free_inodes = set()
		self.allocated_inodes = set()
		self.num_inodes = 0
		self.first_inode_num = 0

		# Use superblock line to determine inode capacity
		with open(csv_file) as opened_csv:
			superblock_line = opened_csv.read().splitlines()[0].split(',')
			self.num_inodes = int(superblock_line[6])
			self.first_inode = int(superblock_line[7])

		# Open file twice because opened_csv gets mutated
		with open(csv_file) as opened_csv:
			reader = csv.DictReader(opened_csv, fieldnames=['name','number'])
			for row in reader:
				if row['name'] == 'IFREE':
					self.free_inodes.add(row['number'])
				elif row['name'] == 'INODE':
					self.allocated_inodes.add(row['number'])

	def check_allocation(self):
		"""Check that all inodes are either allocated or on freelist"""
		for node in self.allocated_inodes:
			if node in self.free_inodes:
				print("ALLOCATED INODE " + str(node) + " ON FREELIST")
				exit_value = 2

		# Inodes between 2 and inodes.first_inode are reserved
		all_inodes = map(str, [2] + list(range(self.first_inode, self.num_inodes + 1)))
		for node in all_inodes:
			if not (node in self.free_inodes or node in self.allocated_inodes):
				print("UNALLOCATED INODE " + str(node) + " NOT ON FREELIST")
				exit_value = 2

	def audit_consistency(self):
		"""Report to stdout all inconsistencies"""
		self.check_allocation()


class directory_tracker:
	def __init__(self, csv_file):
		"""Collect all data needed for consistency audit"""
		self.inode_links = dict() # inode_num -> [discovered_links, link_count]
		self.unallocated_inodes = set()
		self.num_inodes = 0
		self.referenced_inodes = dict()
		self.directory_entries = list()
		self.directory_links = dict()

		with open(csv_file) as opened_csv:
			group_line = opened_csv.read().splitlines()[1].split(',')
			self.num_inodes = int(group_line[3])

		with open(csv_file) as opened_csv:
			entry_lines = opened_csv.read().splitlines()
			for line in entry_lines:
				if "INODE" in line:
					inode_line = line.split(',')
					if inode_line[0] == "INODE":
						if inode_line[1] not in self.inode_links:
							self.inode_links[inode_line[1]] = [0,0]
						self.inode_links[inode_line[1]][1] = inode_line[6]
				elif "IFREE" in line:
					free_inode = line.split(',')
					if free_inode[0] == "IFREE":
						self.unallocated_inodes.add(int(free_inode[1]))
				elif "DIRENT" in line:
					dirent_line = line.split(',')
					if dirent_line[0] == "DIRENT":
						if dirent_line[3] not in self.inode_links:
							self.inode_links[dirent_line[3]] = [0,0]
						self.inode_links[dirent_line[3]][0] += 1
						self.referenced_inodes[int(dirent_line[3])] = (dirent_line[1],dirent_line[6])
						self.directory_entries.append(dirent_line)
						if dirent_line[1] not in self.directory_links:
							self.directory_links[dirent_line[1]] = list()
						self.directory_links[dirent_line[1]].append(dirent_line[3])

	def check_linkcounts(self):
		"""Check that inode reference count equals number of inodes that reference it"""
		for key, value in self.inode_links.items():
			if int(value[1]) == 0:
				continue
			if int(value[0]) != int(value[1]):
				print("INODE " + key + " HAS ", end='')
				print(str(value[0]) + " LINKS BUT LINKCOUNT IS " + str(value[1]))
				exit_value = 2

	def check_inode_status(self):
		"""Check that entries refer to valid and allocated inodes"""
		for key, value in self.referenced_inodes.items():
			if key == 2:
				continue
			if key < 1 or key > self.num_inodes or key in self.unallocated_inodes:
				error = "INVALID"
				if key in self.unallocated_inodes:
					error = "UNALLOCATED"
				print("DIRECTORY INODE " + str(value[0]), end='')
				print(" NAME " + str(value[1]) + " " + error + " INODE " + str(key))
				exit_value = 2

	def check_directory_links(self):
		"""Check that each directory has one link to itself and one link to parent"""
		# Check curr directory links
		for dirent in self.directory_entries:
			if dirent[6] == "'.'":
				if dirent[1] != dirent[3]:
					print("DIRECTORY INODE " + str(dirent[1]), end='')
					print(" NAME " + dirent[6] + " LINK TO INODE ", end='')
					print(dirent[3] + " SHOULD BE " + dirent[1])
					exit_value = 2

		# Check parent links
		for dirent in self.directory_entries:
			if dirent[6] == "'..'":
				# Find parent
				if int(dirent[1]) == 2:
					if int(dirent[3]) != 2:
						print("DIRECTORY INODE " + str(dirent[1]), end='')
						print(" NAME " + dirent[6] + " LINK TO INODE ", end='')
						print(dirent[3] + " SHOULD BE 2")
						exit_value = 2
					continue
				parent_value = 0
				for parent in self.directory_links:
					if parent == dirent[1]:
						continue
					for child in self.directory_links[parent]:
						if child == dirent[1]:
							parent_value = parent
				if dirent[3] != parent_value:
					print("DIRECTORY INODE " + str(dirent[1]), end='')
					print(" NAME " + dirent[6] + " LINK TO INODE ", end='')
					print(dirent[3] + " SHOULD BE " + parent)
					exit_value = 2

	def audit_consistency(self):
		"""Report to stdout all inconsistencies"""
		self.check_linkcounts()
		self.check_inode_status()
		self.check_directory_links()
		

def extract_cl_args():
	"""Parse command line arguments and return name of passed csv file"""
	CL_PARSER = argparse.ArgumentParser()
	CL_PARSER.add_argument("csv_file",
						   help="A csv file containing a file system summary")
	try:
		CL_ARGS = CL_PARSER.parse_args()
	except SystemExit:
		sys.exit(1)

	if not os.path.exists(CL_ARGS.csv_file):
		print("ERROR: File does not exist", file=sys.stderr)
		sys.exit(1)

	return CL_ARGS.csv_file


def main():
	csv_file = extract_cl_args()

	blocks = block_tracker(csv_file)
	blocks.audit_consistency()

	inodes = inode_tracker(csv_file)
	inodes.audit_consistency()

	directories = directory_tracker(csv_file)
	directories.audit_consistency()

	sys.exit(exit_value)
	

if __name__=="__main__":
	main()