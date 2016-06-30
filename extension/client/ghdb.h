/*
	Copyright (C) 2016 CHAN Gonghao.
	
	This Program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License, version 3,
	as published by the Free Software Foundation.

	This Program is distributed in the hope that is will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTSBILITY or FITNESS FOR A PARTICULAR PUPOSE. See the 
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU AFFERO General Public License
	along with this program. If not, see <http://www.gnu.org/license>.
*/
#ifndef _GHDB_HPP_
#define _GHDB_HPP_

#include "core.hpp"
#include "ossSocket.hpp"
#include "commandFactory.hpp"

const int CMD_BUFFER_SIZE = 512 ;
class Ghdb {
   public :
	Ghdb () {}
	~Ghdb () {} ;
   public :
	void	start ( void ) ;
   protected :
	void	prompt ( void ) ;
   private :
	void 	split ( const std::string &text, char delim, std::vector<std::string> &result ) ;
	char* 	readLine ( char *p, int length ) ;
	int 	readInput ( const char *pPrompt, int numIndent ) ;
   private :
	ossSocket 	_sock ;
	CommandFactory 	_cmdFactory ;
	char 		_cmdBuffer [ CMD_BUFFER_SIZE ] ;
}; 

#endif
