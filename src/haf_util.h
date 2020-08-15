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
#include <cstdint>
//
//	TYPES
//
typedef struct 
{
	uint32_t  dirs_num;
	uint32_t  dirs_pointer;
	char	bank_type[2];
	char	bank_version;
} header_t;

typedef struct
{
	uint32_t  chunk_pointer;
	uint32_t  chunk_size;
	char    chunk_name[10];
} directory_t;

//
//	FUNCTIONS
//

// Create an empty bank file
FILE* HUF_CreateEmpty   ( const char* filename, header_t** out_header );

// Load header from The Bank File to our_header 
// IF bank doesn't exist create new empety one an return new header object
bool HUF_LoadHeader    ( 
							const char*  filename, 
							header_t**   out_header
						);

// Load all Chunk Directories into a table (array) out_dir_table
bool HUF_LoadDirsTable ( 
							const char*   filename,       header_t*		 header,
							directory_t** out_dirs_table, uint32_t*      toatl_chunk_size
						);
/*void HUF_LoadDirsTable (const char* filename, header_t* out_header 
						 , directory_t* out_dirs_table);
						 */

// Load Chunk form The Bank File
bool HUF_LoadChunk     (
							const char* filename,         header_t*    header ,
							const char* chunk_name,       directory_t* dirs_table,
							char*       out_chunk_buffer, uint32_t*         out_chunk_size
					   );

bool HUF_AddChunk      (
							const char* filename,   header_t*	 header, 
							const char* chunk_name, char *		 chunk_buffer, 
							uint32_t      chunk_size, directory_t** dirs_table
					   );

bool HUF_DeleteChunk   (
							const char* filename,   header_t*    header,
							const char* chunk_name, directory_t* dirs_table
	             	   );

// Return latest Error 
const char* HUF_GetError();

// Return lastest operation execution time (ms)
long HUF_GetExcutionTime();
			