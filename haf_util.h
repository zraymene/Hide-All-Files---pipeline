/**
 *  Copyright 2020 by Zeroual Aymene <ayemenzeroual@gmail.com>
 *
 *  Licensed under GNU General Public License 3.0 or later.
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 * FILE: haf_util.h
 *
 * DECRP:
 *	- Functions declarations 
 *  
 */

#pragma once

#include <cstdio>

//
//	TYPES
//
typedef struct 
{

	int  dirs_num;
	int  dirs_pointer;
	char bank_type[2];
	char bank_version;

} header_t;

typedef struct
{
	int  chunk_pointer;
	int  chunk_size;
	char chunk_name[8];

} directory_t;

//
//	FUNCTIONS
//

// Load header from The Bank File to our_header 
bool HUF_LoadHeader    ( 
							const char* filename, 
							header_t**   out_header,
							std::FILE**        out_file
						);

// Load all Chunk Directories into a table (array) out_dir_table
bool HUF_LoadDirsTable ( 
							const char*  filename,
							directory_t* out_dirs_table,
							long*         toatl_chunk_size
						);
/*void HUF_LoadDirsTable (const char* filename, header_t* out_header 
						 , directory_t* out_dirs_table);
						 */

// Load Chunk form The Bank File
bool HUF_LoadChunk     (
							const char* filename,         header_t*    header ,
							const char* chunk_name,       directory_t* dirs_table,
							char*       out_chunk_buffer, int*         out_chunk_size
					   );

bool HUF_AddChunk      (
							const char* filename,   header_t* header, 
							const char* chunk_name, char *    chunk_buffer, 
							int         chunk_size, directory_t* dirs_table
					   );

bool HUF_DeleteChunk   (
							const char* filename,   header_t*    header,
							const char* chunk_name, directory_t* dirs_table
	             	   );

// Return latest Error 
const char* HUF_GetError();

// Make sure you call this before exisitng or you will get memory leak !
void		HUF_Exit();						