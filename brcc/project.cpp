
/*  o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o

    CTool Library
    Copyright (C) 1998-2001	Shaun Flisakowski

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o  */
/*  o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    o+
    o+     File:         project.cpp
    o+
    o+     Programmer:   Shaun Flisakowski
    o+     Date:         Nov 27, 1998
    o+
    o+     TransUnit and Project classes.
    o+
    o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o  */

#include <cstdio>
#include <cstring>
#include <cassert>
#include <fstream>

#include "project.h"

#include "express.h"
#include "stemnt.h"

#include "token.h"
extern int err_cnt;
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o

#ifdef    USE_GCC_4_CPP
#undef    LIB_CPP
#define   LIB_CPP    "gcc -E "
#endif

#ifdef    USE_CAT_4_CPP
#undef    LIB_CPP
#define   LIB_CPP    "cat "
#endif

#ifdef    USE_CPP_4_CPP
#ifndef   LIB_CPP
#define   LIB_CPP    "/lib/cpp "
#endif
#endif

#ifdef	  USE_CL_4_CPP
#undef    LIB_CPP
#define   LIB_CPP    "cl /P /I\"e:\\Program Files\\Microsoft Visual Studio\\VC98\\Include\" "
#endif

#ifndef   LIB_CPP
#define   LIB_CPP    "/lib/cpp "
#endif	

/* o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o */

extern std::istream *yyinstream;

extern int yyparse(void);

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o

char what_version[] = "@(#) " VERSION;

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o

Project *gProject = NULL;

bool Project::gDebug = false;
// Fill in StdPath[1] with path obtained from running cpp on <stddef.h>
char  *StdPath[] = { "/usr/include/", NULL, NULL };

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
InitStdPath( const char* cpp_cmd /*=(const char *) NULL */ )
{
#ifdef	USE_CL_4_CPP
#	define    INCL_EXAMPLE    "incl.c"
#else
#	define    INCL_EXAMPLE    "incl.h"
#endif
    {
        std::ofstream inclOut(INCL_EXAMPLE);
        inclOut << "#include <stddef.h>\n";
    }

    Project prj;

    TransUnit *unit = prj.parse(INCL_EXAMPLE, true, cpp_cmd);

#ifdef  DEBUG_CEA
    std::cerr << std::endl
              << "Std include path[0]: \"" << StdPath[0] << "\"" << std::endl;
#endif

    if (unit)
    {
        for (Statement *stemnt = unit->head; stemnt; stemnt = stemnt->next)
            if (stemnt->isInclude())
            {
                InclStemnt *incl = (InclStemnt*) stemnt;
    
                char *inclPath = new char [strlen(incl->filename.c_str())+1];
                char *inclEnd;
    
                strcpy(inclPath,incl->filename.c_str());

#ifdef	USE_CL_4_CPP
				if ((inclEnd = strrchr(inclPath, '\\')))
#else
                if ((inclEnd = strrchr(inclPath, '/')))
#endif
                {
                    inclEnd++;
                    *inclEnd = '\0';
                    StdPath[1] = inclPath;
#ifdef  DEBUG_CEA
                    std::cerr << "Std include path[1]: \""
                              << StdPath[1] << "\"" << std::endl;
#endif
                }
                else
                    delete [] inclPath;
        
                break;
            }
    }

    remove(INCL_EXAMPLE);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
TransUnit::TransUnit(const std::string& fname)
 :filename(fname)
{
    head = tail = NULL;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
TransUnit::~TransUnit()
{
    Statement *ste, *nxt = NULL;

    for (ste=head; ste; ste=ste->next)
    {
        delete nxt;
        nxt = ste;
    }

    delete nxt;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
TransUnit::findStemnt( fnStemntCallback cb )
{
    Statement *ste;

    for (ste=head; ste; ste=ste->next)
    {
        ste->findStemnt(cb);
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
TransUnit::findFunctionDef( fnFunctionCallback cb )
{
    Statement *ste;

    for (ste=head; ste; ste=ste->next)
    {
        // Function definition can only occur at the top level.
        if (ste->isFuncDef())
            (cb)((FunctionDef*) ste);
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
TransUnit::add( Statement *stemnt )
{
    if (stemnt)
    { 
        stemnt->next = NULL;
    
        if (tail)
            tail->next = stemnt;
        else
            head = stemnt;
    
        tail = stemnt;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
TransUnit::insert(Statement *stemnt, Statement *after /* =NULL */)
{
    if (stemnt)
    {
        stemnt->next = NULL;

        if (tail)
        {
            if (after)
            {
                stemnt->next = after->next;
                after->next = stemnt;
            }
            else
            {
                stemnt->next = head;
                head = stemnt;
            }

            if (stemnt->next == NULL)
                tail = stemnt;
        }
        else
            head = tail = stemnt;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
std::ostream&
operator<<(std::ostream& out, const TransUnit& tu)
{
	int	inInclude = 0;
    Statement *stemnt;

    for (stemnt=tu.head; stemnt; stemnt=stemnt->next)
	{
		if (inInclude > 0)
		{
			if (stemnt->isEndInclude())
				inInclude--;
			else if (stemnt->isInclude())
				inInclude++;
		}
		else
		{
			if (stemnt->isInclude())
				inInclude++;

        	out << *stemnt << std::endl;
		}
	}

    return out;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Project::Project()
  : units()
{
    typeList = NULL;
}

Project::~Project()
{
    TransUnitVector::iterator    j;
    for (j=units.begin(); j != units.end(); j++)
    {
        delete *j;
    }

    Type::DeleteTypeList(typeList);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// Parse a file and add it to the list of stored TransUnits
TransUnit*
Project::parse( const char* path, bool use_cpp, /* =true */
                const char* cpp_dir,     /* =(const char *)NULL */
                bool keep_cpp_file,       /* =false */
                const char* cpp_outputfile, /* =(const char *)NULL */
                const char* cpp_cmd,     /* =(const char *)NULL */
                const char* cd_cmd       /* ="cd % ;"           */ )
{
    std::ifstream   fp;
    char       cpp_cmmd[500];
    char       cpp_file[200];
    char       *end;
    
    gProject = this;

    strcpy(cpp_file, path);
    end = strrchr(cpp_file,'.');

    if (!end)
    {
      std::cerr << "No '.' found in filename '" << path << "'\n";
      return NULL;
    }

#ifdef  DEBUG_CEA
    std::cerr << std::endl << "Parsing file: \"" << cpp_file << "\"" << std::endl;
#endif

#ifdef	USE_CL_4_CPP
    if (use_cpp) 
        sprintf(end, ".i");
#else
    if (use_cpp) 
        sprintf(end, "_pp");
#endif
 
/*
  willey
    need to redirect output to file if using GCC, not give target file
    as final argument, as in cpp
*/

    if (use_cpp)
    {
      if (cpp_outputfile)
         strcpy(cpp_file, cpp_outputfile);

      if (cpp_dir)
        sprintf(cpp_cmmd, cd_cmd, cpp_dir);

      if (cpp_cmd)
        sprintf(cpp_cmmd, cpp_cmd, path, cpp_file);
      else

#ifdef  USE_GCC_4_CPP
        sprintf(cpp_cmmd,"%s -DCTOOL -D__STDC__ %s > %s",LIB_CPP, path, cpp_file);

#elif defined(USE_CL_4_CPP)
		sprintf(cpp_cmmd,"%s /DCTOOL /D__STDC__ %s",LIB_CPP, path);

#elif defined(USE_CAT_4_CPP)

        sprintf(cpp_cmmd,"%s %s > %s", LIB_CPP, path, cpp_file);
#else
        sprintf(cpp_cmmd, "%s -DCTOOL -D__STDC__ %s %s", LIB_CPP, path, cpp_file);
#endif

#ifdef  DEBUG_CEA
        /* debugging line to see what preprocessor is being called */
        std::cerr << cpp_cmmd << std::endl;
#endif

        system(cpp_cmmd);
    }
 
    fp.open(cpp_file);

    if (!fp)
        return NULL;
    
    Parse_TOS = new ParseEnv(&fp, &std::cerr, path);
    TransUnit *unit = Parse_TOS->transUnit;
    
    yyinstream = Parse_TOS->yyinstream;

    unit->contxt.EnterScope();

    while(yyparse())
        ;

    fp.close();

    if (use_cpp && !keep_cpp_file && remove(cpp_file))
    {
      std::cerr << "Remove of file '" << cpp_file << "' failed.\n";
    }

    /* Build the translation unit. */
    delete gProject->Parse_TOS;
    gProject->Parse_TOS = NULL;


    if (unit == NULL)
        return NULL;
 
    unit->contxt.ExitScope();

    if (! err_cnt)
       units.push_back(unit);
    else
    {
       if (gDebug)
            std::cout << *unit << std::endl;
       delete unit;
       unit = NULL;
    }
    return unit;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
