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
#include "commandFactory.hpp"

CommandFactory::CommandFactory(){
	addCommand ();
}

ICommand * CommandFactory::getCommandProcessor ( const char * pCmd ) {
	ICommand *pProcessor = NULL ;
	do {
		COMMAND_MAP::iterator iter ;
		iter = _cmdMap.find( pCmd ) ;
		if ( iter != _cmdMap.end() )
			pProcessor = iter -> second ;
	}while( 0 ) ;
	return pProcessor ;
}
