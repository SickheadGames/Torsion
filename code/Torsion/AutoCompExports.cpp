// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompExports.h"

#include "AutoCompUtil.h"
#include "Markup.h"

#include <wx/regex.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


AutoCompExports::AutoCompExports( const wxString& name )
   :  m_Name( name ),
      m_Classes( CmpNameNoCase ),
      m_Functions( CmpNameNoCase ),
      m_InStream( NULL ),
      m_TextStream( NULL )
{
}

AutoCompExports::AutoCompExports( const AutoCompExports& exports )
   :  m_Name( exports.m_Name ),
      m_Classes( CmpNameNoCase ),
      m_Functions( CmpNameNoCase ),
      m_InStream( NULL ),
      m_TextStream( NULL )
{
   for ( int i=0; i < exports.m_Classes.GetCount(); i++ )
      m_Classes.Add( new AutoCompClass( *exports.m_Classes[i] ) );
   for ( int i=0; i < exports.m_Functions.GetCount(); i++ )
      m_Functions.Add( new AutoCompFunction( *exports.m_Functions[i] ) );
}

AutoCompExports::~AutoCompExports()
{
   wxDELETE( m_TextStream );
   wxDELETE( m_InStream );

   WX_CLEAR_ARRAY( m_Classes );
   WX_CLEAR_ARRAY( m_Functions );
}

bool AutoCompExports::LoadFromDoxygen( const wxString& path )
{
   // TODO: Add a callback for checking progress and aborting!
   m_InStream = new wxFileInputStream( path );
   if ( !m_InStream->Ok() )
   {
      wxDELETE( m_InStream );
      return false;
   }

   m_TextStream = new wxTextInputStream( *m_InStream, " \t\r\n" );

   // Pre-compile the expressions used here.
   wxRegEx nsExpr;
   nsExpr.Compile( "[ \t]*namespace[ \t]*(.*?)[ \t][{]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( nsExpr.IsValid() );
   wxRegEx derivedClassExpr;
   derivedClassExpr.Compile( "[ \t]*class[ \t]*(.*?)[ \t][:][ \t]*public[ \t]*(.*?)[ \t]]*[{]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( derivedClassExpr.IsValid() );
   wxRegEx classExpr;
   classExpr.Compile( "[ \t]*class[ \t]*(.*?)[ \t]*[{]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( classExpr.IsValid() );

   // Pre-compile the expressions used in
   // the functions we call from here.
   m_CommentExpr.Compile( "[ \t]*[/][*][!]?(.*?)([*][/])?", wxRE_ADVANCED | wxRE_ICASE );
   //m_CommentExpr.Compile( "[ \t]*[/][*][!](.*?)[*][/]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( m_CommentExpr.IsValid() );
   m_FuncExpr.Compile( "[ \t]*(virtual)[ \t]+(.*?)[ \t]+(.*?)[\\(](.*?)[\\)][ \t]+[{}]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( m_FuncExpr.IsValid() );
   m_EndExpr.Compile( "[ \t]*[}][;]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( m_EndExpr.IsValid() );

   // Loop thru the namespaces and classes.
   wxString line;
   while ( !m_InStream->Eof() ) {

      // Read line by line looking for the 
      // start of namespace and class blocks.
      line = m_TextStream->ReadLine();
      if ( line.IsEmpty() )
         continue;

      // Look for the "gloabl" namespace.
      if (  nsExpr.Matches( line ) && 
            nsExpr.GetMatch( line, 1 ).CmpNoCase( "global" ) == 0 ) {

         _ReadDoxygenGlobalMethods();
         continue;
      }

      // Look for classes with bases.
      else if (  derivedClassExpr.Matches( line ) &&
            derivedClassExpr.GetMatchCount() == 3 ) {

         wxString name = derivedClassExpr.GetMatch( line, 1 );
         wxString base = derivedClassExpr.GetMatch( line, 2 );

         _ReadDoxygenClass( name, base );
         continue;
      }

      // Just look for a regular class now.
      else if ( classExpr.Matches( line ) ) {

         wxString name = classExpr.GetMatch( line, 1 );

         _ReadDoxygenClass( name, wxEmptyString );
         continue;
      }     
   }

   // Fixup classes... remove vars and functions that are
   // already defined in their base classes.  This happens
   // normally for vars... seems like a Torque bug.
   for ( int i=0; i < m_Classes.GetCount(); i++ )
   {
      AutoCompClass* class_ = m_Classes[i];
      wxASSERT( class_ );
      
      AutoCompClass* base = class_;
      for (;;)
      {
         base = AutoCompClass::Find( base->GetBase(), m_Classes );
         if ( !base )
            break;

         const AutoCompVarArray& vars = base->GetVars();
         for ( int j=0; j < vars.GetCount(); j++ )
         {
            wxASSERT( vars[j] );
            class_->DeleteVar( vars[j]->GetName() );
         }

         /*
            TODO: I cannot do this for functions because i 
            found at least one case where a class ligitiately
            overloads a base function.  See 'save' in ActionMap
            and SimObject.

         const AutoCompFunctionArray& functions = base->GetFunctions();
         for ( int j=0; j < functions.GetCount(); j++ )
         {
            wxASSERT( functions[j] );
            class_->DeleteFunction( functions[j]->GetName() );
         }
         */
      }
   }

   wxDELETE( m_TextStream );
   wxDELETE( m_InStream );
   return true;
}

bool AutoCompExports::_ReadDoxygenComment( const wxString& line, wxString& comment )
{
   if ( !m_CommentExpr.Matches( line ) )
      return false;

   comment = m_CommentExpr.GetMatch( line, 1 );
   comment.Trim();
   comment.Trim( false );

   // Is this an open comment?
   if ( m_CommentExpr.GetMatch( line, 2 ).IsEmpty() ) 
   {
      wxASSERT( m_InStream && m_InStream->Ok() );
      wxASSERT( m_TextStream );

      // Append lines till we get a closed match.
      wxString line;
      while ( !m_InStream->Eof() ) 
      {
         line = m_TextStream->ReadLine();
         int end = line.Find( "*/" );

         if ( end != -1 ) 
         {
            line = line.Mid( 0, end );

            line.Trim();
            line.Trim( false );
            comment << "\n" << line;

            break;
         }

         line.Trim();
         line.Trim( false );

         comment << "\n" << line;
      }

      comment.Trim();
      comment.Trim( false );
   }

   // We ignore group block comments.
   if ( comment.Find( "@{" ) != -1 )
      comment.Empty();

   return true;
}

void AutoCompExports::_ReadDoxygenClass( const wxString& name, const wxString& base )
{
   wxASSERT( m_InStream && m_InStream->Ok() );
   wxASSERT( m_TextStream );

   wxString lastComment;

   AutoCompClass* class_ = new AutoCompClass( name, base );

   // Pre-allocate the expressions... much faster.
   wxRegEx varExpr;
   varExpr.Compile( "[ \t]*(.*?)[ \t]+(.*?)[;]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( varExpr.IsValid() );
   
   wxString line;
   while ( !m_InStream->Eof() )
   {
      // Read line by line looking for comments, methods, and members.
      line = m_TextStream->ReadLine();
      if ( line.IsEmpty() )
         continue;

       // Is this a comment?
      if ( _ReadDoxygenComment( line, lastComment ) )
         continue;

      // Is this a member function?
      else if ( m_FuncExpr.Matches( line ) ) 
      {
         wxString name = m_FuncExpr.GetMatch( line, 3 );
         if ( AutoCompFunction::Find( name, class_->GetFunctions() ) )
         {
            // TODO: What can we do but ignore it?
         }
         else
         {
            AutoCompFunction* func = class_->AddFunction( name );
            //func->SetReturn( m_FuncExpr.GetMatch( line, 2 ) );
            func->SetArgs( m_FuncExpr.GetMatch( line, 4 ) );
            func->SetDesc( lastComment );
         }

         lastComment.Empty();
         continue;
      }

      // Is this a member var?
      else if ( varExpr.Matches( line ) ) 
      {
         wxString name = varExpr.GetMatch( line, 2 );
         if ( AutoCompVar::Find( name, class_->GetVars() ) )
         {
            // TODO: What can we do but ignore it?
         }
         else
         {
            AutoCompVar* var = class_->AddVar( name );
            var->m_Desc = lastComment;

            // If we don't have a description stuff 
            // the type in there.
            if ( var->m_Desc.IsEmpty() ) {
               var->m_Desc << varExpr.GetMatch( line, 1 ) << " " << var->m_Name;
               var->m_Desc.Trim();
               var->m_Desc.Trim( true );
            }
         }

         lastComment.Empty();
         continue;
      }

      // Is this the end of the method block?
      else if ( m_EndExpr.Matches( line ) )
      {
         // Are we getting a class twice?
         if ( AutoCompClass::Find( name, m_Classes ) )
         {
            // TODO: What can we do but ignore it?
         }
         else
         {
            m_Classes.Add( class_ );
            class_ = NULL;
         }
         break;
      }

      lastComment.Empty();

   } // while ( !m_InStream->Eof() )

   // If we still have the object here then it
   // lacked a close bracket or was a duplicate
   // and it should be added to the exports.
   wxDELETE( class_ );
}

void AutoCompExports::_ReadDoxygenGlobalMethods()
{
   wxASSERT( m_InStream && m_InStream->Ok() );
   wxASSERT( m_TextStream );

   wxString lastComment;

   while ( !m_InStream->Eof() ) 
   {
      // Read line by line looking for comments and functions.
      wxString line = m_TextStream->ReadLine();
      if ( line.IsEmpty() )
         continue;

      // Is this a comment?
      if ( _ReadDoxygenComment( line, lastComment ) )
         continue;

      // Is this a function?
      if ( m_FuncExpr.Matches( line ) ) {
         
         // Make sure we don't already have it.
         wxString name = m_FuncExpr.GetMatch( line, 3 );
         if ( AutoCompFunction::Find( name, m_Functions ) )
         {
            // TODO: What can we do but ignore it?
         }
         else
         {
            AutoCompFunction* func = new AutoCompFunction( name );
            //func->SetReturn( m_FuncExpr.GetMatch( line, 2 ) );
            func->SetArgs( m_FuncExpr.GetMatch( line, 4 ) );
            func->SetDesc( lastComment );
            m_Functions.Add( func );
         }

         lastComment.Empty();
         continue;
      }

      // Is this the end of the method block?
      if ( m_EndExpr.Matches( line ) )
         break;

      lastComment.Empty();

   } // while ( !m_InStream->Eof() )

}

wxString AutoCompExports::MakeExistNote( const wxString& desc, int exportBits, const AutoCompExportsArray& exports )
{
   wxString configs;
   int allConfigBits = 0;

   // Gather the config names this export exists in.
   for ( int i=0; i < exports.GetCount(); i++ ) 
   {
      allConfigBits |= (1<<i);

      if ( exportBits & (1<<i) )
         configs << exports[i]->GetName() << ", ";
   }

   // If the element exists in all configs then 
   // just return the description.
   if ( allConfigBits == exportBits )
      return desc;

   wxString note;
   if ( !desc.IsEmpty() )
      note << desc << "\n";
   note << "(only exists in " << configs;
   note.RemoveLast();
   note.RemoveLast();
   note << ")";

   return note;
}


void AutoCompExports::Merge( const AutoCompExportsArray& exports )
{
   // We must go thru each and every item, in each and every page,
   // and add "only in XXX, YYY, and ZZZ" strings into the end of
   // each description.  This solves the issue, for instance, of
   // displaying autocomplete information for functions that only
   // exist in the debug build configuration.

   for ( int i=0; i < exports.GetCount(); i++ ) {

      AutoCompClassArray& classes = exports[i]->m_Classes;
      while ( !classes.IsEmpty() ) {

         // This will be the same size and have the
         // same order as the page array, so we can
         // easily lookup each object's page name.
         AutoCompUnsortedClassArray exported;
         exported.SetCount( exports.GetCount(), NULL );

         AutoCompClass* class_ = classes[0];
         classes.Remove( class_ );
         wxASSERT( exported.Index( class_ ) == -1 );
         exported[i] = class_;

         int exportBits = 1 << i;

         // Remove this class from the other exports.
         for ( int k=0; k < exports.GetCount(); k++ ) {
            
            if ( k == i )
               continue;

            AutoCompClassArray& matchClasses = exports[k]->m_Classes;
            AutoCompClass* match = AutoCompClass::Find( class_->GetName(), matchClasses );
            if ( match ) {
               exportBits |= 1 << k;
               wxASSERT( matchClasses.Index( match ) != -1 );
               matchClasses.Remove( match );
               wxASSERT( exported.Index( match ) == -1 );

               if ( class_->GetDesc().IsEmpty() )
                  class_->SetDesc( match->GetDesc() );

               exported[k] = match;
            }
         }

         // If the object doesn't exist in all pages then
         // we need to add a notation of what configs it
         // does exist in.
         class_->SetDesc( AutoCompExports::MakeExistNote( class_->GetDesc(), exportBits, exports ) );

         // Now we must merge together the members of the objects.
         class_->MergeMembers( exported, exports );

         // Add the object to us and delete the others.
         wxASSERT( m_Classes.Index( class_ ) == -1 );
         m_Classes.Add( class_ );
         wxASSERT( exported.Index( class_ ) != -1 );
         exported.Remove( class_ );
         WX_CLEAR_ARRAY( exported );
      }

      AutoCompFunctionArray& functions = exports[i]->m_Functions;
      while ( !functions.IsEmpty() ) {

         AutoCompFunction* func = functions[0];
         functions.Remove( func );

         int exportBits = 1 << i;

         // Remove this function from the other pages.
         for ( int k=0; k < exports.GetCount(); k++ ) {
            
            if ( k == i )
               continue;

            AutoCompFunction* match = AutoCompFunction::Find( func->GetName(), exports[k]->m_Functions );
            if ( match ) {
               exportBits |= 1 << k;
               exports[k]->m_Functions.Remove( match );

               if ( func->GetArgs().IsEmpty() )
                  func->SetArgs( match->GetArgs() );

               if ( func->GetDesc().IsEmpty() )
                  func->SetDesc( match->GetDesc() );

               delete match;
            }
         }

         // If the object doesn't exist in all pages then
         // we need to add a notation of what configs it
         // does exist in.
         func->SetDesc( AutoCompExports::MakeExistNote( func->GetDesc(), exportBits, exports ) );

         // Add the function to the page.
         m_Functions.Add( func );
      }
   }

   // Mark the datablocks.
   AutoCompClass::SetDatablocks( &m_Classes );
}


bool AutoCompExports::SaveXml( const wxString& path ) const
{
	wxFile File;
	if ( !File.Open( path, wxFile::write ) )
		return false;

   CMarkup xml;

   xml.AddElem( L"exports" );
   xml.IntoElem();

   _SaveClasses( xml, m_Classes );
   _SaveFunctions( xml, "function", m_Functions );

   xml.OutOfElem();

   const std::wstring& buffer = xml.GetDoc();
	bool noerror = File.Write( buffer.c_str(), buffer.length() ) == buffer.length();

   File.Close();
   return noerror;
}

void AutoCompExports::_SaveClasses( CMarkup& xml, const AutoCompClassArray& classes )
{
   for ( int i=0; i < classes.GetCount(); i++ ) {

      AutoCompClass* class_ = classes[i];
      wxASSERT( class_ );

      xml.AddElem(L"class" );
      xml.IntoElem();

      wxASSERT( !class_->GetName().IsEmpty() );

      xml.AddElem(L"name", class_->GetName().wc_str() );
      if ( !class_->GetBase().IsEmpty() )
         xml.AddElem(L"base", class_->GetBase().wc_str() );
      if ( !class_->GetDesc().IsEmpty() )
         xml.AddElem(L"desc", class_->GetDesc().wc_str() );

      _SaveFunctions( xml, "method", class_->GetFunctions() );
      _SaveVars( xml, "field", class_->GetVars() );

      xml.OutOfElem();
   }
}

void AutoCompExports::_SaveFunctions( CMarkup& xml, const wxString& elem, const AutoCompFunctionArray& functions )
{
   for ( int i=0; i < functions.GetCount(); i++ ) {

      AutoCompFunction* func = functions[i];
      wxASSERT( func );

      xml.AddElem( elem.wc_str() );
      xml.IntoElem();

      wxASSERT( !func->GetName().IsEmpty() );

      xml.AddElem(L"name", func->GetName().wc_str() );

      //if ( !func->GetReturn().IsEmpty() )
      //   xml.AddElem( "return", func->GetReturn() );
      if ( !func->GetArgs().IsEmpty() )
         xml.AddElem(L"args", func->GetArgs().wc_str() );
      if ( !func->GetDesc().IsEmpty() )
         xml.AddElem(L"desc", func->GetDesc().wc_str() );

      xml.OutOfElem();
   }
}


void AutoCompExports::_SaveVars( CMarkup& xml, const wxString& elem, const AutoCompVarArray& vars )
{
   for ( int i=0; i < vars.GetCount(); i++ ) {

      AutoCompVar* var = vars[i];
      wxASSERT( var );

      xml.AddElem( elem.wc_str() );
      xml.IntoElem();

      wxASSERT( !var->m_Name.IsEmpty() );
      xml.AddElem(L"name", var->m_Name.wc_str() );

      if ( !var->m_Desc.IsEmpty() )
         xml.AddElem(L"desc", var->m_Desc.wc_str() );

      xml.OutOfElem();
   }
}

bool AutoCompExports::LoadXml( const wxString& path )
{
	wxFile File;
	if ( !File.Open( path, wxFile::read ) ) {
      return false;
	}

   size_t Length = File.Length();
   wchar_t* Buffer = new wchar_t[ Length+1 ];
	File.Read( Buffer, Length );
	Buffer[ Length ] = 0;
   CMarkup xml;
   bool error = xml.SetDoc( Buffer );
	delete [] Buffer;   

   if ( !error ) {
      wxASSERT_MSG( error, xml.GetError().c_str() );
      return false;
   }

   if ( !xml.FindElem(L"exports" ) )
      return false;

   xml.IntoElem();

   xml.ResetMainPos();
   _LoadClasses( xml, m_Classes );
   AutoCompClass::SetDatablocks( &m_Classes );

   xml.ResetMainPos();
   _LoadFunctions( xml, "function", m_Functions );

   return true;
}

void AutoCompExports::_LoadClasses( CMarkup& xml, AutoCompClassArray& objects )
{
   // Now grab the classes.
   while ( xml.FindElem(L"class" ) )
   {
      xml.IntoElem();

      // Get the class name... else there is nothing to add.
      if ( !xml.FindElem(L"name" ) )
      {
         xml.OutOfElem();
         continue;
      }

      wxString name = xml.GetData().c_str();
      if ( name.IsEmpty() ) 
      {
         xml.OutOfElem();
         continue;
      }

      wxString base;
      xml.ResetMainPos();
      if ( xml.FindElem(L"base" ) )
         base = xml.GetData().c_str();

      // Do we already have this one?
      if ( AutoCompClass::Find( name, objects ) )
      {
         // TODO: What can be do but skip it?
         xml.OutOfElem();
         continue;
      }

      // Add it.
      AutoCompClass* object = new AutoCompClass( name, base );
      objects.Add( object );

      // Look for a description.
      xml.ResetMainPos();
      if ( xml.FindElem(L"desc" ) )
         object->SetDesc( xml.GetData().c_str() );

      // Load the methods.
      xml.ResetMainPos();
      AutoCompFunctionArray functions( CmpNameNoCase );
      _LoadFunctions( xml, "method", functions );
      object->AddFunctions( &functions );
      wxASSERT( functions.IsEmpty() );

      // Load member vars.
      xml.ResetMainPos();
      AutoCompVarArray vars( CmpNameNoCase );
      _LoadVars( xml, "field", vars );
      object->AddVars( &vars );
      wxASSERT( vars.IsEmpty() );

      xml.OutOfElem();

   } // while ( xml.FindElem( elem.c_str() ) )
}

void AutoCompExports::_LoadFunctions( CMarkup& xml, const wxString& elem, AutoCompFunctionArray& functions )
{
   while ( xml.FindElem( elem.wc_str() ) )
   {
      xml.IntoElem();

      if ( !xml.FindElem(L"name" ) )
      {
         xml.OutOfElem();
         continue;
      }

      wxString name = xml.GetData().c_str();
      if ( name.IsEmpty() ) 
      {
         xml.OutOfElem();
         continue;
      }

      // Do we already have this one?
      if ( AutoCompFunction::Find( name, functions ) )
      {
         // TODO: What can be do but skip it?
         xml.OutOfElem();
         continue;
      }

      AutoCompFunction* func = new AutoCompFunction( name );
      functions.Add( func );

      //xml.ResetMainPos();
      //if ( xml.FindElem( "return" ) )
      //   func->SetReturn( xml.GetData().c_str() );

      xml.ResetMainPos();
      if ( xml.FindElem(L"args" ) )
         func->SetArgs( xml.GetData().c_str() );

      xml.ResetMainPos();
      if ( xml.FindElem(L"desc" ) )
         func->SetDesc( xml.GetData().c_str() );

      xml.OutOfElem();

   } // while ( xml.FindElem( elem.c_str() ) )
}

void AutoCompExports::_LoadVars( CMarkup& xml, const wxString& elem, AutoCompVarArray& vars )
{
   while ( xml.FindElem( elem.wc_str() ) ) {

      xml.IntoElem();

      if ( !xml.FindElem( L"name" ) ) {
         xml.OutOfElem();
         continue;
      }

      wxString name = xml.GetData().c_str();
      if ( name.IsEmpty() ) {
         xml.OutOfElem();
         continue;
      }

      // Do we already have this one?
      if ( AutoCompVar::Find( name, vars ) )
      {
         // TODO: What can be do but skip it?
         xml.OutOfElem();
         continue;
      }

      AutoCompVar* var = new AutoCompVar( name );
      vars.Add( var );

      xml.ResetMainPos();
      if ( xml.FindElem(L"desc" ) )
         var->m_Desc = xml.GetData().c_str();

      xml.OutOfElem();

   } // while ( xml.FindElem( elem.c_str() ) )
}

void AutoCompExports::GetFunctionsString( wxString* output, wxChar sep ) const
{
   wxASSERT( output );

   // Take a guess at the size.
   output->Alloc( output->Length() + ( m_Functions.GetCount() * 11 ) ); 

   // Get all the function names.
   for ( size_t i=0; i < m_Functions.GetCount(); i++ ) 
   {
      AutoCompFunction* func = m_Functions[i];
      wxASSERT( func ); 
      output->Append( func->GetName() );
      output->Append( sep );
   }

   output->RemoveLast();
   output->Shrink();
}

void AutoCompExports::GetClassString( wxString* output, wxChar sep ) const
{
   wxASSERT( output );

   // Take a guess at the size.
   output->Alloc( output->Length() + ( m_Classes.GetCount() * 11 ) ); 

   // Get all the class names.
   for ( size_t i=0; i < m_Classes.GetCount(); i++ ) 
   {
      AutoCompClass* class_ = m_Classes[i];
      wxASSERT( class_ ); 
      output->Append( class_->GetName() );
      output->Append( sep );
   }

   output->RemoveLast();
   output->Shrink();
}

void AutoCompExports::GetVarsString( wxString* output, wxChar sep ) const
{
   wxASSERT( output );

   // TODO: Fix Torque to return exported vars!
}


