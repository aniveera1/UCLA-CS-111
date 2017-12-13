/*
 * NAME: Anirudh Veeraragavan,Rahul Sheth
 * EMAIL: aveeraragavan@g.ucla.edu,rahulssheth@g.ucla.edu
 * ID: 004767663,304779669
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

// Global Constants
int SUCCESS_CODE = 0;
int ERR_CODE = 1;
int FAIL_CODE = 2;
int STDOUT = 1;

int SUPERBLOCK_OFFSET = 1024;
int SUPERBLOCK_SIZE = 1024;
int GROUP_DESC_SIZE = 32;
int INODE_SIZE = 128;

// Global Variables
struct superblock {
	u_int32_t block_count;
	u_int32_t inode_count;
	u_int32_t block_size;
	u_int16_t inode_size;
	u_int32_t blocks_per_group;
	u_int32_t inodes_per_group;
	u_int32_t first_inode;
};

struct blockgroup {
	u_int32_t group_id;
	u_int32_t num_blocks;
	u_int32_t num_inodes;
	u_int16_t free_block_count;
	u_int16_t free_inodes_count;
	u_int32_t block_bitmap;
	u_int32_t inode_bitmap;
	u_int32_t inode_table;
};

struct inode {
	int32_t inode_num;
	char file_type;
	u_int16_t mode;
	u_int32_t owner;
	u_int32_t group;
	u_int16_t link_count;
	u_int32_t time_since_inode_change;
	u_int32_t modification_time;
	u_int32_t time_last_access;
	u_int32_t file_size;
	u_int32_t num_blocks;
	u_int32_t offset;
	u_int32_t block_addresses[15];
};

struct directory {
	int32_t parent_inode;
	u_int32_t logical_offset;
	u_int32_t referenced_inode;
	u_int16_t entry_len;
	u_int8_t name_len;
	u_int8_t name[255];
};

// INPUT: Name of sys call that threw error
// Prints reason for error and terminates program
void process_failed_sys_call(const char syscall[])
{
	int err = errno;
	fprintf(stderr, "%s", "An error has occurred.\n");
	fprintf(stderr, "The system call '%s' failed with error code %d\n", syscall, err);
	fprintf(stderr, "This error code means: %s\n", strerror(err));
	exit(FAIL_CODE);
}

// INPUT: Command line arguments
// Extract image file and return file descriptor
int process_cl_arguments(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s\n", "ERROR: Wrong number of arguments.");
		fprintf(stderr, "%s\n", "USAGE: lab3a [img_file]");
		exit(ERR_CODE);
	}
	char* img_file = argv[1];

	char* type = ".img";
	if (strstr(img_file, type) == NULL)
	{
		fprintf(stderr, "%s\n", "ERROR: Wrong argument.");
		fprintf(stderr, "%s\n", "USAGE: lab3a [img_file]");
		exit(ERR_CODE);
	}

	int img_fd = open(img_file, O_RDONLY);
	if (img_fd < 0)
	{
		process_failed_sys_call("open");
	}

	return img_fd;
}

// INPUT: File system image file descriptor and struct to store superblock data in
// Extract superblock summary
void extract_superblock_info(int img_fd, struct superblock* superblock_data)
{
	// Total number of blocks
	int bytes_read = pread(img_fd, &superblock_data->block_count, 4, SUPERBLOCK_OFFSET + 4);
	if (bytes_read < 0)
	{
		process_failed_sys_call("pread");
	}

	// Total number of inodes
	bytes_read = pread(img_fd, &superblock_data->inode_count, 4, SUPERBLOCK_OFFSET);
	if (bytes_read < 0)
	{
		process_failed_sys_call("pread");
	}

	// Block size
	u_int32_t buf;
	bytes_read = pread(img_fd, &buf, 4, SUPERBLOCK_OFFSET + 24);
	if (bytes_read < 0)
	{
		process_failed_sys_call("pread");
	}
	superblock_data->block_size = 1024 << buf;

	// Inode size
	bytes_read = pread(img_fd, &superblock_data->inode_size, 2, SUPERBLOCK_OFFSET + 88);
	if (bytes_read < 0)
	{
		process_failed_sys_call("pread");
	}

	// Blocks per group
	bytes_read = pread(img_fd, &superblock_data->blocks_per_group, 4, SUPERBLOCK_OFFSET + 32);
	if (bytes_read < 0)
	{
		process_failed_sys_call("pread");
	}

	// Inodes per group
	bytes_read = pread(img_fd, &superblock_data->inodes_per_group, 4, SUPERBLOCK_OFFSET + 40);
	if (bytes_read < 0)
	{
		process_failed_sys_call("pread");
	}

	// First non-reserved inode
	bytes_read = pread(img_fd, &superblock_data->first_inode, 4, SUPERBLOCK_OFFSET + 84);
	if (bytes_read < 0)
	{
		process_failed_sys_call("pread");
	}
}

// INPUT: File system image file descriptor, superblock data, blockgroup data, number of groups
// Extract block group data
void extract_blockgroup_data(int img_fd, struct superblock* superblock_data, struct blockgroup* group_data, u_int32_t num_block_groups)
{
	unsigned int remaining_inodes = superblock_data->inode_count;
	unsigned int remaining_blocks = superblock_data->block_count;

	unsigned int i;
	for (i = 0; i < num_block_groups; i++)
	{
		// Group number
		group_data[i].group_id = i;

		// Number of blocks in group
		if (remaining_blocks > superblock_data->blocks_per_group)
		{
			group_data[i].num_blocks = superblock_data->blocks_per_group;
			remaining_blocks -= superblock_data->blocks_per_group;
		}
		else
		{
			group_data[i].num_blocks = remaining_blocks;
		}

		// Number of inodes in group
		if (remaining_inodes > superblock_data->inodes_per_group)
		{
			group_data[i].num_inodes = superblock_data->inodes_per_group;
			remaining_inodes -= superblock_data->inodes_per_group;
		}
		else
		{
			group_data[i].num_inodes = remaining_inodes;
		}

		// Number of free blocks
		int bytes_read = pread(img_fd, &group_data[i].free_block_count, 2, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i * GROUP_DESC_SIZE) + 12);
		if (bytes_read < 0)
		{
			process_failed_sys_call("pread");
		}

		// Number of free inodes
		bytes_read = pread(img_fd, &group_data[i].free_inodes_count, 2, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i * GROUP_DESC_SIZE) + 14);
		if (bytes_read < 0)
		{
			process_failed_sys_call("pread");
		}

		// Block number of free block bitmap
		bytes_read = pread(img_fd, &group_data[i].block_bitmap, 4, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i * GROUP_DESC_SIZE));
		if (bytes_read < 0)
		{
			process_failed_sys_call("pread");
		}

		// Block number of free inode bitmap
		bytes_read = pread(img_fd, &group_data[i].inode_bitmap, 4, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i * GROUP_DESC_SIZE) + 4);
		if (bytes_read < 0)
		{
			process_failed_sys_call("pread");
		}

		// Block number of first block of inodes
		bytes_read = pread(img_fd, &group_data[i].inode_table, 4, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i * GROUP_DESC_SIZE) + 8);
		if (bytes_read < 0)
		{
			process_failed_sys_call("pread");
		}
	}
}

// INPUT: File system img, superblock data, blockgroup data, number of groups, block bitmaps
// Extract block bitmaps
void extract_block_bitmaps(int img_fd, struct superblock* superblock_data, struct blockgroup* group_data,
						   u_int32_t num_block_groups, u_int32_t bytes_block_bitmap, u_int8_t* block_bitmaps)
{
	unsigned int i;
	unsigned int j;
	for (i = 0; i < num_block_groups; i++)
	{
		for (j = 0; j < bytes_block_bitmap; j++)
		{
			int bytes_read = pread(img_fd, &block_bitmaps[i * bytes_block_bitmap + j], 1, superblock_data->block_size * group_data[i].block_bitmap + j);
			if (bytes_read < 0)
			{
				process_failed_sys_call("pread");
			}
		}
	}
}

// INPUT: File system img, superblock data, blockgroup data, number of groups, inode bitmaps
// Extract inode bitmaps
void extract_inode_bitmaps(int img_fd, struct superblock* superblock_data, struct blockgroup* group_data,
						   u_int32_t num_block_groups, u_int8_t* inode_bitmaps, u_int32_t bytes_inode_bitmap)
{
	unsigned int i;
	unsigned int j;
	for (i = 0; i < num_block_groups; i++)
	{
		for (j = 0; j < bytes_inode_bitmap; j++)
		{
			int bytes_read = pread(img_fd, &inode_bitmaps[i * bytes_inode_bitmap + j], 1, superblock_data->block_size * group_data[i].inode_bitmap + j);
			if (bytes_read < 0)
			{
				process_failed_sys_call("pread");
			}
		}
	}
}

// INPUT: Epoch time
// Prints time representation mm/dd/yy hh:mm:ss to STDOUT
void print_gmt_time(u_int32_t time)
{
	time_t timer = (time_t)time;
	struct tm time_info;
	gmtime_r(&timer, &time_info);

	dprintf(STDOUT, "%02d/", time_info.tm_mon + 1);
	dprintf(STDOUT, "%02d/", time_info.tm_mday);
	dprintf(STDOUT, "%d ", time_info.tm_year - 100);
	dprintf(STDOUT, "%d:", time_info.tm_hour);
	dprintf(STDOUT, "%d:", time_info.tm_min);
	dprintf(STDOUT, "%d,", time_info.tm_sec);
}

// Extract inode data
void extract_inode_data(int img_fd, struct superblock* superblock_data, struct blockgroup* group_data,
						u_int32_t num_block_groups, u_int8_t* inode_bitmaps, u_int32_t bytes_inode_bitmap,
						struct inode* inode_data)
{
	unsigned int i;
	unsigned int j;
	unsigned int p;
	for (i = 0; i < num_block_groups; i++)
	{
		for (j = 0; j < bytes_inode_bitmap; j++)
		{
			u_int8_t bitmap = inode_bitmaps[i * bytes_inode_bitmap + j];
			for (p = 1; p <= 8; p++)
			{
				inode_data[(j * 8) + p - 1].inode_num = -1;

				int inode_allocated = 1 & bitmap;

				u_int16_t inode_mode = 0;
				pread(img_fd, &inode_mode, 2, (superblock_data->block_size * group_data[i].inode_table) + (((j * 8) + p - 1) * INODE_SIZE) + 0);

				u_int16_t link_count = 0;
				pread(img_fd, &link_count, 2, (superblock_data->block_size * group_data[i].inode_table) + (((j * 8) + p - 1) * INODE_SIZE) + 26);

				if (inode_allocated == 1 && inode_mode != 0 && link_count != 0)
				{
					int index = (j * 8) + p - 1;

					// Offset
					inode_data[index].offset = (superblock_data->block_size * group_data[i].inode_table) + (((j * 8) + p - 1) * INODE_SIZE);

					// Inode Number
					inode_data[index].inode_num = (j * 8) + p;

					// File type
					if (inode_mode & 0x8000)
					{
						inode_data[index].file_type = 'f';
					}
					else if (inode_mode & 0x4000)
					{
						inode_data[index].file_type = 'd';
					}
					else if (inode_mode & 0xA000)
					{
						inode_data[index].file_type = 's';
					}
					else
					{
						inode_data[index].file_type = '?';
					}
					
					// Inode Mode
					inode_data[index].mode = inode_mode;

					// Owner
					u_int16_t uid;
					pread(img_fd, &uid, 2, inode_data[index].offset + 2);

					u_int16_t uid_high;
					pread(img_fd, &uid_high, 2, inode_data[index].offset + 116 + 4);

					inode_data[index].owner = (uid_high << 16) | uid;

					// Group
					u_int16_t gid;
					pread(img_fd, &gid, 2, inode_data[index].offset + 24);

					u_int16_t gid_high;
					pread(img_fd, &gid_high, 2, inode_data[index].offset + 116 + 6);

					inode_data[index].group = (gid_high << 16) | gid;

					// Link Count
					inode_data[index].link_count = link_count;

					// Time of last inode change
					pread(img_fd, &inode_data[index].time_since_inode_change, 4, inode_data[index].offset + 12);
					
					// Modification Time
					pread(img_fd, &inode_data[index].modification_time, 4, inode_data[index].offset + 16);

					// Time of last access
					pread(img_fd, &inode_data[index].time_last_access, 4, inode_data[index].offset + 8);

					// File Size
					pread(img_fd, &inode_data[index].file_size, 4, inode_data[index].offset + 4);

					// Number of blocks
					pread(img_fd, &inode_data[index].num_blocks, 4, inode_data[index].offset + 28);

					// Block addresses
					unsigned int a;
					for (a = 0; a < 15; a++)
					{
						pread(img_fd, &inode_data[index].block_addresses[a], 4, inode_data[index].offset + 40 + (a * 4));
					}
				}
				bitmap = bitmap >> 1;
			}
		}
	}
}


int main(int argc, char** argv)
{
	int img_fd = process_cl_arguments(argc, argv);

	// Superblock summary
	struct superblock superblock_data;
	extract_superblock_info(img_fd, &superblock_data);

	dprintf(STDOUT, "%s", "SUPERBLOCK,");
	dprintf(STDOUT, "%d,", superblock_data.block_count);
	dprintf(STDOUT, "%d,", superblock_data.inode_count);
	dprintf(STDOUT, "%d,", superblock_data.block_size);
	dprintf(STDOUT, "%d,", superblock_data.inode_size);
	dprintf(STDOUT, "%d,", superblock_data.blocks_per_group);
	dprintf(STDOUT, "%d,", superblock_data.inodes_per_group);
	dprintf(STDOUT, "%d\n", superblock_data.first_inode);


	// Blockgroup Summary
	// Divide block count by blocks per group to get num of groups
	// Use pigeonhole principle for round up division
	u_int32_t num_block_groups = (superblock_data.block_count + superblock_data.blocks_per_group - 1) / superblock_data.blocks_per_group;

	struct blockgroup* group_data = (struct blockgroup *) malloc(sizeof(struct blockgroup) * num_block_groups);
	extract_blockgroup_data(img_fd, &superblock_data, group_data, num_block_groups);

	unsigned int i;
	for (i = 0; i < num_block_groups; i++)
	{
		dprintf(STDOUT, "%s,", "GROUP");
		dprintf(STDOUT, "%d,", group_data[i].group_id);
		dprintf(STDOUT, "%d,", group_data[i].num_blocks);
		dprintf(STDOUT, "%d,", group_data[i].num_inodes);
		dprintf(STDOUT, "%d,", group_data[i].free_block_count);
		dprintf(STDOUT, "%d,", group_data[i].free_inodes_count);
		dprintf(STDOUT, "%d,", group_data[i].block_bitmap);
		dprintf(STDOUT, "%d,", group_data[i].inode_bitmap);
		dprintf(STDOUT, "%d\n", group_data[i].inode_table);
	}


	// Free Block Summary
	u_int32_t bytes_block_bitmap = superblock_data.blocks_per_group / 8;
	u_int8_t* block_bitmaps = (u_int8_t *) malloc(sizeof(u_int8_t) * num_block_groups * bytes_block_bitmap);
	
	extract_block_bitmaps(img_fd, &superblock_data, group_data, num_block_groups, 
						  bytes_block_bitmap, block_bitmaps);

	unsigned int j;
	unsigned int p;
	for (i = 0; i < num_block_groups; i++)
	{
		for (j = 0; j < bytes_block_bitmap; j++)
		{
			u_int32_t bitmap = block_bitmaps[i * bytes_block_bitmap + j];
			for (p = 1; p <= 8; p++)
			{
				int block_allocated = 1 & bitmap;
				if (block_allocated == 0)
				{
					dprintf(STDOUT, "%s,", "BFREE");
					dprintf(STDOUT, "%d\n", (j * 8) + p);
				}
				bitmap = bitmap >> 1;
			}
		}
	}


	// Free Inode Summary
	u_int32_t bytes_inode_bitmap = superblock_data.inodes_per_group / 8;
	u_int8_t* inode_bitmaps = (u_int8_t *) malloc(sizeof(u_int8_t) * num_block_groups * bytes_inode_bitmap);
	
	extract_inode_bitmaps(img_fd, &superblock_data, group_data, num_block_groups,
						  inode_bitmaps, bytes_inode_bitmap);

	unsigned int num_nodes = 0;
	for (i = 0; i < num_block_groups; i++)
	{
		for (j = 0; j < bytes_inode_bitmap; j++)
		{
			u_int8_t bitmap = inode_bitmaps[i * bytes_inode_bitmap + j];
			for (p = 1; p <= 8; p++)
			{
				int inode_allocated = 1 & bitmap;
				if (inode_allocated == 0)
				{
					dprintf(STDOUT, "%s,", "IFREE");
					dprintf(STDOUT, "%d\n", (j * 8) + p);
				}
				else
				{
					num_nodes++;
				}
				bitmap = bitmap >> 1;
			}
		}
	}


	// Inode Summary
	struct inode* inode_data = (struct inode *) malloc(sizeof(struct inode) * num_nodes);

	extract_inode_data(img_fd, &superblock_data, group_data, num_block_groups,
					   inode_bitmaps, bytes_inode_bitmap, inode_data);

	for (i = 0; i < num_nodes; i++)
	{
		if (inode_data[i].inode_num != -1)
		{
			dprintf(STDOUT, "%s,", "INODE");
			dprintf(STDOUT, "%d,", inode_data[i].inode_num);
			dprintf(STDOUT, "%c,", inode_data[i].file_type);
			dprintf(STDOUT, "%o,", inode_data[i].mode & 0xFFF);
			dprintf(STDOUT, "%d,", inode_data[i].owner);
			dprintf(STDOUT, "%d,", inode_data[i].group);
			dprintf(STDOUT, "%d,", inode_data[i].link_count);
			print_gmt_time(inode_data[i].time_since_inode_change);
			print_gmt_time(inode_data[i].modification_time);
			print_gmt_time(inode_data[i].time_last_access);
			dprintf(STDOUT, "%d,", inode_data[i].file_size);
			dprintf(STDOUT, "%d,", inode_data[i].num_blocks);
			for (j = 0; j < 14; j++)
			{
				dprintf(STDOUT, "%d,", inode_data[i].block_addresses[j]);
			}
			dprintf(STDOUT, "%d\n", inode_data[i].block_addresses[14]);
		}
	}


	// Directory Entries
	for (i = 0; i < num_nodes; i++)
	{
		if (inode_data[i].inode_num != -1 && inode_data[i].file_type == 'd')
		{
			u_int32_t directory_file_block;
			pread(img_fd, &directory_file_block, 4, inode_data[i].offset + 40);

			u_int32_t curr_directory_entry_inode;
			pread(img_fd, &curr_directory_entry_inode, 4, directory_file_block * superblock_data.block_size + 0);

			u_int32_t directory_file_offset = 0;
			while (curr_directory_entry_inode != 0 && directory_file_offset < 1024)
			{
				u_int16_t entry_len;
				pread(img_fd, &entry_len, 2, directory_file_block * superblock_data.block_size + directory_file_offset + 4);

				u_int8_t name_len;
				pread(img_fd, &name_len, 1, directory_file_block * superblock_data.block_size + directory_file_offset + 6);

				u_int8_t name[255];
				memset(name, 0, sizeof(name));
				pread(img_fd, name, name_len, directory_file_block * superblock_data.block_size + directory_file_offset + 8);

				dprintf(STDOUT, "%s,", "DIRENT");
				dprintf(STDOUT, "%d,", inode_data[i].inode_num);
				dprintf(STDOUT, "%d,", directory_file_offset);
				dprintf(STDOUT, "%d,", curr_directory_entry_inode);
				dprintf(STDOUT, "%d,", entry_len);
				dprintf(STDOUT, "%d,", name_len);
				dprintf(STDOUT, "'%s'\n", name);

				directory_file_offset += entry_len;
				pread(img_fd, &curr_directory_entry_inode, 4, directory_file_block * superblock_data.block_size + directory_file_offset + 0);

			}
		}
	}


	// Indirect Block references
	for (i = 0; i < num_nodes; i++)
	{
		if (inode_data[i].inode_num != -1 && (inode_data[i].file_type == 'f' || inode_data[i].file_type == 'd'))
		{
			for (j = 12; j < 15; j++)
			{
				u_int32_t indirect_block;
				pread(img_fd, &indirect_block, 4, inode_data[i].offset + 40 + (j * 4));

				if (indirect_block == 0)
				{
					continue;
				}

				for (p = 0; p < superblock_data.block_size / 4; p++)
				{
					u_int32_t next_block;
					pread(img_fd, &next_block, 4, indirect_block * superblock_data.block_size + 4 * p);

					if (next_block == 0)
					{
						continue;
					}

					u_int32_t block_one;
					if (j == 12)
					{
						block_one = 12 + p;
					}
					else if (j == 13)
					{
						block_one = 12 + 256 + p;
					}
					else
					{
						block_one = 12 + 256 + 256 * 256 + p;
					}

					printf("INDIRECT,%d,%d,%d,%d,%d\n", inode_data[i].inode_num, j - 11, block_one, indirect_block, next_block);

					if (j > 12)
					{
						u_int32_t n;
						for (n = 0; n < superblock_data.block_size / 4; n++) 
                        {
                        	u_int32_t second_block;
                            pread(img_fd, &second_block, 4, next_block * superblock_data.block_size + 4 * n);

                            if (second_block == 0)
                            {
                            	continue;
                            }

                            u_int32_t block_two;
                            if (j == 13) 
                            {
                                block_two = 12 + 256 + n;
                            } 
                            else 
                            {
                                block_two = 12 + 256 + 256 * 256+ n;
                                
                            }
                            printf("INDIRECT,%d,%d,%d,%d,%d\n", inode_data[i].inode_num, j - 12, block_two, next_block, second_block);
                            
                            if (j > 13) 
                            {
                            	u_int32_t m;
                                for (m = 0; m < superblock_data.block_size / 4; m++) 
                                {
                                	u_int32_t third_block;
                                    pread(img_fd, &third_block, 4, second_block * superblock_data.block_size + 4 * m);

                                    if (third_block == 0)
                                    {
                                    	continue;
                                    }

                                    u_int32_t block_three = 12 + 256 + 256 * 256 + m;
                                    printf("INDIRECT,%d,%d,%d,%d,%d\n", inode_data[i].inode_num, j - 13, block_three, second_block, third_block);
                                }
                            }
                        }
					}
				}
			}
		}
	}

	free(inode_data);
	free(inode_bitmaps);
	free(block_bitmaps);
	free(group_data);

	close(img_fd);

	exit(SUCCESS_CODE);
}