/**
 *  Copyright 2020 by Zeroual Aymene <ayemenzeroual@gmail.com>
 *
 *  Licensed under GNU General Public License 3.0 or later.
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 * FILE: haf_util.cpp
 *
 * DECRP:
 *	- Functions difintions
 *  - tiny error handling system
 *
 * NOTE:
 *	- M_ prefixes means it's member of this unit and can't be acces from outside
 *
 */

#include "haf_util.h"

#include <cstring>

char* ERROR_STR;

void M_PushError(const char* error_msg)
{
	if (ERROR_STR != nullptr)
		delete[] ERROR_STR;

	ERROR_STR = new char[strlen(error_msg)];

}

bool M_FetchDirectory(directory_t* dir_table, int size , const char* wanted_dir_name , directory_t** out_wanted_dir , int * out_dir_positon)
{

	for (int i = 0; i < size; i++)
	{
		if (strcmp((dir_table + i)->chunk_name, wanted_dir_name))
		{
			*out_wanted_dir = (dir_table + i);

			if(out_dir_positon != nullptr)
				*out_dir_positon = i ;

			return true;
		}
	}

	return false;

}

//
// Opens and loades the header file
//
// TODO:
//  - Check file opening permissions (w , r, ...) if it rewrite 
//    exisintg file or something 
//
bool HUF_LoadHeader(const char* filename, header_t** out_header , std::FILE ** out_file)
{

	if ((*out_file = std::fopen(filename, "r")) == nullptr)
	{
		M_PushError("HUF Error : Can't open The Bank File !");
		return false;
	}

	if(*out_header == nullptr)
		*out_header = new header_t();

	// Read The Bank header in header_t struct
	if( std::fread(*out_header, sizeof(header_t), 1, *out_file) == 0 )
	{
		M_PushError("HUF Error : Error while reading Bank header !");
		return false;
	}

	return true;
}

//
// Loads Directories Table  + calculate total chunks size
//
// TODO :
//  - Check for the best way of reading directories table ( commented )
//
bool HUF_LoadDirsTable(const char* filename, directory_t* out_dirs_table , long* toatl_chunk_size)
{

	std::FILE * file;
	header_t* header;
	*toatl_chunk_size = 0;

	if (!HUF_LoadHeader(filename,&header,&file) )
		return false;

	// Alloc array to hold all directories
	out_dirs_table = new directory_t[sizeof(directory_t) * header->dirs_num];

	for (int i = 0; i < header->dirs_num; i++)
	{

		if (std::fseek( file,
						(i * sizeof(directory_t)) + header->dirs_pointer,
			            SEEK_SET) != 0)
		{
			M_PushError("HUF Error : Can't seek to directories position !");
			return false;
		}

		if (std::fread(&out_dirs_table[i], sizeof(directory_t), 1, file) == 0)
		{
			M_PushError("HUF Error : Can't read directories !");
			return false;
		}

		// Calculate all chunks size
		*toatl_chunk_size += out_dirs_table[i].chunk_size;

	}

	// Or you this 
	/*
	std::fseek(file, header->dirs_pointer, SEEK_SET);
	std::fread(out_dirs_table, sizeof(directory_t), header->dirs_pointer, file);
	*/

	std::fclose(file);

	return true;
}

//
// Load a desired chunk by directory name , return chunk buffer + chunk size
//
bool HUF_LoadChunk(
					const char* filename,          header_t*    header ,
					const char* wanted_chunk_name, directory_t* dirs_table,
					char*       out_chunk_buffer,  int*         out_chunk_size
				  )
{

	std::FILE* file;
	directory_t* dir_wanted;

	if (!HUF_LoadHeader(filename, &header, &file))
		return false;

	if (!M_FetchDirectory(dirs_table, header->dirs_num, wanted_chunk_name , &dir_wanted , nullptr))
	{
		M_PushError("HUF Error : Can't find desired chunk/directory !");
		return false;
	}

	out_chunk_buffer = new char[dir_wanted->chunk_size];
	*out_chunk_size = dir_wanted->chunk_size;

	if (std::fseek(file, dir_wanted->chunk_pointer, SEEK_SET) != 0)
	{
		M_PushError("HUF Error : Can't seek to chunk position !");
		return false;
	}

	if (fread(out_chunk_buffer, dir_wanted->chunk_size, 1, file) == 0)
	{
		M_PushError("HUF Error : Can't read chunk position !");
		return false;
	}

	delete dir_wanted;

	fclose(file);

	return true;
}

//
// Add new chunk to The Bank File 
// Return new header , dirs_table (As args of course)
// 
// TODO : Memory leaks is highly possible hir , recheck !
//
bool HUF_AddChunk(
					const char* filename,   header_t*	 header,
					const char* chunk_name, char*		 chunk_buffer,
					int         chunk_size, directory_t* dirs_table
				 )
{

	std::FILE* file;

	if (!HUF_LoadHeader(filename, &header, &file))
		return false;

	// Look if directory with same name but we don't need any return value !
	if (M_FetchDirectory(dirs_table, header->dirs_num, chunk_name, nullptr , nullptr))
	{
		M_PushError("HUF Error : Chunk with same name already exists ! !");
		return false;
	}

	// New directory 
	directory_t* dir_tmp = new directory_t();
	strcpy(dir_tmp->chunk_name, chunk_name);
	dir_tmp->chunk_size = chunk_size;
	dir_tmp->chunk_pointer = header->dirs_pointer - 1;

	// Buffer to hold all existing directories plus a place for new directory
	int dirs_tmp_buff_size = (header->dirs_num + 1) * sizeof(directory_t);
	directory_t* dirs_tmp_buffer = new directory_t[dirs_tmp_buff_size];

	// Copy old table to the new !
	for (int i = 0; i < header->dirs_num; i++)
	{
		*(dirs_tmp_buffer + i) = *(dirs_table);
	}

	// Add new directory to then end of dirs table
	*(dirs_tmp_buffer + header->dirs_num) = *dir_tmp;

	// Free old table
	delete dirs_table;
	// Now it's the new table !
	dirs_table = dirs_tmp_buffer;

	//###################################################
	// Go to new chunk position
	//###################################################
	if (std::fseek(file, dir_tmp->chunk_pointer, SEEK_SET) != 0)
	{
		M_PushError("HUF Error : Can't seek to new chunk position !");
		return false;
	}
	// Write the new chunk !
	if (std::fwrite(chunk_buffer, chunk_size, 1, file) <= 0)
	{
		M_PushError("HUF Error : Error while writing new chunk ! !");
		return false;
	}

	//#####################################################
	// New directories tabke turn , seek to the new positon
	//#####################################################
	if (std::fseek(file, dir_tmp->chunk_pointer + ( dir_tmp->chunk_size - 1) , SEEK_SET) != 0)
	{
		M_PushError("HUF Error : Can't seek to new directories table position !");
		return false;
	}
	// Write the new table !
	if (std::fwrite(dirs_tmp_buffer, dirs_tmp_buff_size,  (header->dirs_num) + 1, file) <= 0)
	{
		M_PushError("HUF Error : Error while writing new table ! !");
		return false;
	}

	//#####################################################
	// New header turn !!
	//#####################################################
	header->dirs_num += 1;
	header->dirs_pointer += (chunk_size - 1);
	// Go to the begining !
	if (std::fseek(file, 0, SEEK_SET) != 0)
	{
		M_PushError("HUF Error : Can't seek to the header positon !");
		return false;
	}
	// Now overwrite !
	if (std::fwrite(header, sizeof(header_t) , 1, file) <= 0)
	{
		M_PushError("HUF Error : Error while writing new header ! !");
		return false;
	}

	fclose(file);

	return true;
}

//
//	Delete a desired chunk by directory name , returns updated header , directories table 
//
// TODO: Leeks hir are huge whene there is an error !
//
bool HUF_DeleteChunk(
						const char* filename,   header_t* header,
						const char* chunk_name, directory_t* dirs_table
					)
{

	std::FILE* file;
	std::FILE* file2;

	directory_t* wanted_dir = nullptr;
	int			 wanted_dir_pos;

	if (!HUF_LoadHeader(filename, &header, &file))
		return false;

	// Look if directory exists !
	if (!M_FetchDirectory(dirs_table, header->dirs_num, chunk_name, nullptr , &wanted_dir_pos))
	{
		M_PushError("HUF Error : Chunk desired to delete doesn't exists ! !");
		return false;
	}

	//#####################################################
	// First part of chunks , before desired chunk!!
	//#####################################################
	// Calc first buffer size  from end of header -> start of desired chunk 
	// Then allocate 
	long buff1_size = wanted_dir->chunk_pointer - sizeof(header_t);
	char* buff1     = new char[buff1_size];
	//-----------------------------------------------------
	// No seek to end of header part 
	if (std::fseek(file, sizeof(header_t) , SEEK_SET) != 0)
	{
		M_PushError("HUF Error - Delete : Can't seek to the end of header part !");
		return false;
	}
	// Read until start of desired chunk !
	if (fread(buff1 , sizeof(char) , buff1_size , file) == 0)
	{
		M_PushError("HUF Error - Delete : Can't read chunks to buffer1 !");
		return false;
	}

	//#####################################################
	// Second part of data - chunks , after desired chunk!!
	//#####################################################
	// Calc second buffer size  from end of desired chunk -> start of desired chunk directory
	// Then allocate 
	long buff2_begin = wanted_dir->chunk_pointer + wanted_dir->chunk_size;
	long buff2_end   = header->dirs_pointer + (wanted_dir_pos * ( sizeof(directory_t) - 1 ) );
	long buff2_size  = buff2_end - buff2_begin;
	char* buff2      = new char[buff2_size];
	//-----------------------------------------------------
	// Now seek to end of header part 
	if (std::fseek(file, buff2_begin , SEEK_SET) != 0)
	{
		M_PushError("HUF Error - Delete : Can't seek to the end of desired chunk  !");
		return false;
	}
	// Read until start of desired chunk directory !
	if (fread(buff2, sizeof(char), buff2_size, file) == 0)
	{
		M_PushError("HUF Error - Delete : Can't read chunks to buffer2 !");
		return false;
	}
	
	// Both parts are done , good now write them to the new file
	// Before edit header 
	// Set the new directires table position
	header->dirs_num -= 1;
	header->dirs_pointer -= wanted_dir->chunk_size; 
	
	char tmp_file_name[13] = "tmp-bank.bnk";

	if ( (file2 = std::fopen(tmp_file_name, "w")) == nullptr)
	{
		M_PushError("HUF Error - Delete : Can't open temporary bank file !");
		return false;
	}

	//-----------------------------------------------------
	// Write new header
	//-----------------------------------------------------
	/*if (std::fseek(file2, 0, SEEK_SET) != 0)
	{
		M_PushError("HUF Error - Delete : Can't seek to the start of file!");
		return false;
	}
	*/
	if (std::fwrite(header, sizeof(header_t), 1, file2) <= 0)
	{
		M_PushError("HUF Error - Delete : Can't write header to new bank file!");
		return false;
	}

	//-----------------------------------------------------
	// Write first buffer
	//-----------------------------------------------------
	/*if (std::fseek(file2, sizeof(header_t) , buff1_size , SEEK_SET) != 0)
	{
		M_PushError("HUF Error - Delete : Can't seek to the end of header - writing to TMP!");
		return false;
	}
	*/
	if (std::fwrite(buff1, sizeof(char), buff1_size, file2) <= 0)
	{
		M_PushError("HUF Error - Delete : Can't write first buffer - writing to TMP!");
		return false;
	}
	delete[] buff1;

	//-----------------------------------------------------
	// Write second buffer
	//-----------------------------------------------------
	/*if (std::fseek(file2, sizeof(header_t) + buff1_size - 1, buff2_size , SEEK_SET) != 0)
	{
		M_PushError("HUF Error - Delete : Can't seek to the end of header - writing to TMP!");
		return false;
	}
	*/
	if (std::fwrite(buff2, sizeof(char), buff2_size, file2) <= 0)
	{
		M_PushError("HUF Error - Delete : Can't write second buffer - writing to TMP!");
		return false;
	}
	delete[] buff2;

	//#####################################################
	// Third part of data - chunks , after desired chunk directory!!
	//#####################################################
	// First check if desired chunk directory isn't the last or exit
	if (wanted_dir_pos != (header->dirs_num - 1))
	{

		if (std::fseek(file2, 0, SEEK_SET) != 0)
		{
			M_PushError("HUF Error - Delete : Can't seek to the end of header - writing to TMP!");
			return false;
		}

		long buff3_end   = std::ftell(file2); // Get the end of the file
		long buff3_begin = buff2_end + sizeof(directory_t);
		long buff3_size = buff3_end - buff3_begin;

		char* buff3 = new char[buff3_size];

		//-----------------------------------------------------
		// Seek to end of desired chunk directory
		if (std::fseek(file, buff2_begin, SEEK_SET) != 0)
		{
			M_PushError("HUF Error - Delete : Can't seek to the end of desired chunk directory !");
			return false;
		}
		// Read until start of desired chunk directory !
		if (fread(buff3, sizeof(char), buff3_size, file) == 0)
		{
			M_PushError("HUF Error - Delete : Can't read chunks to buffer3 !");
			return false;
		}

		//-----------------------------------------------------
		// Write third buffer
		//-----------------------------------------------------
		/*if (std::fseek(file2, sizeof(header_t) + buff1_size - 1, buff2_size , SEEK_SET) != 0)
		{
			M_PushError("HUF Error - Delete : Can't seek to the end of header - writing to TMP!");
			return false;
		}
		*/
		if (std::fwrite(buff3, sizeof(char), buff3_size, file2) <= 0)
		{
			M_PushError("HUF Error - Delete : Can't write third buffer - writing to TMP!");
			return false;
		}

		delete[] buff3;
	}

	// Phewww , finaly!
	// Now delete original file
	// Then rename the tmp file to the original name
	fclose(file);
	fclose(file2);

	std::remove(filename);
	std::rename(tmp_file_name, filename);

	// Noice it took 2 hours to write this 
	// My butt is stuck on the chair !!

	return true;
}

const char* HUF_GetError()
{
	return ERROR_STR;
}

void HUF_Exit()
{
	if (ERROR_STR != nullptr)
		delete[] ERROR_STR;
}

