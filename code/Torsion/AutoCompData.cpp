// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompData.h"

#include "AutoCompPage.h"
#include "AutoCompExports.h"
#include "AutoCompUtil.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


const char* AutoCompData::IDENT_VAR             = "?0";
const char* AutoCompData::IDENT_FUNCTION        = "?1";
const char* AutoCompData::IDENT_CLASS           = "?2";
const char* AutoCompData::IDENT_DATABLOCK       = "?3";
const char* AutoCompData::IDENT_OBJECT          = "?4";
const char* AutoCompData::ITYPE_DATABLOCKOBJECT = "?5";
const char* AutoCompData::IDENT_NAMESPACE       = "?6";
const char* AutoCompData::ITYPE_PACKAGE         = "?7";
const char* AutoCompData::ITYPE_KEYWORD         = "?8";


AutoCompData::AutoCompData()
   :  m_Classes( CmpNameNoCase ),
      m_Objects( CmpNameNoCase ),
      m_Functions( CmpNameNoCase ),
      m_Vars( CmpNameNoCase ),
      m_Files( CmpNameNoCase ), // TODO: Is sorting no-case an issue here?
      m_Namespaces( CmpNameNoCase ),
      m_Exports( NULL )
{
}

AutoCompData::~AutoCompData()
{
   Clear();
}

bool AutoCompData::HasData() const
{
   return   !m_Classes.IsEmpty() ||
            !m_Functions.IsEmpty() ||
            !m_Objects.IsEmpty() ||
            !m_Vars.IsEmpty() ||
            !m_Namespaces.IsEmpty();
}

void AutoCompData::Clear()
{
   m_Classes.Empty();
   m_Functions.Empty();
   m_Objects.Empty();
   m_Vars.Empty();

   wxDELETE( m_Exports );
   WX_CLEAR_ARRAY( m_Namespaces );
   WX_CLEAR_ARRAY( m_Files );
}

void AutoCompData::Build( const AutoCompPageArray& files, AutoCompExports* exports )
{
   Clear();

   // Copy in the export data first... this ensures that
   // we overload exports if something in the scripts
   // has the same name.
   if ( exports )
   {
      m_Exports = new AutoCompExports( *exports );

      const AutoCompClassArray& classes = m_Exports->GetClasses();
      for ( int i=0; i < classes.GetCount(); i++ )
      {
         wxASSERT( classes[i] );
         wxASSERT( !FindClassOrObject( classes[i]->GetName() ) );
         m_Classes.Add( classes[i] );
      }

      const AutoCompFunctionArray& functions = m_Exports->GetFunctions();
      for ( int i=0; i < functions.GetCount(); i++ )
      {
         wxASSERT( functions[i] );
         wxASSERT( !AutoCompFunction::Find( functions[i]->GetName(), m_Functions ) );
         m_Functions.Add( functions[i] );
      }
   }

   // Make a copy of the files.
   for ( int i=0; i < files.GetCount(); i++ )
   {
      wxASSERT( files[i] );
      m_Files.Add( new AutoCompPage( *files[i] ) );
   }

   // Now we merge the objects and vars into our global
   // arrays... this allows for quickly finding data
   // without the need to dig thru pages.
   for ( int i=0; i < m_Files.GetCount(); i++ ) 
   {
      wxASSERT( m_Files[i] );

      MergeObjects( m_Files[i]->GetObjects() );
      MergeVars( m_Files[i]->GetVars() );
   }

   // Now do a pass merging functions so that they are properly
   // made into members of existing objects and classes or added
   // into new namespaces.
   for ( int i=0; i < m_Files.GetCount(); i++ ) 
   {
      wxASSERT( m_Files[i] );
      MergeFunctions( m_Files[i]->GetFunctions() );
   }

   // Now we build the string lists.
   wxString name;
   m_CompList.Empty();
   m_CompIndex.Empty();
   m_ClassList.Empty();
   m_ClassIndex.Empty();
   m_DatablockList.Empty();
   m_DatablockIndex.Empty();
   m_GlobalsList.Empty();
   m_GlobalsIndex.Empty();

   //
   // Grab all the global vars for autocomplete
   // triggered by entering a $.
   //
   m_TempArray.Empty();
   for ( int i=0; i < m_Vars.GetCount(); i++ )
   {
      wxASSERT( m_Vars[i]->IsGlobal() );

      name = m_Vars[i]->m_Name + IDENT_VAR;

      m_TempArray.Add( name );
   }

   BuildString( m_TempArray, &m_GlobalsList, &m_GlobalsIndex );

   //
   // Gather all the concrete datablock types for 
   // autocomplete when the type name is needed.
   //
   m_TempArray.Empty();
   for ( int i=0; i < m_Classes.GetCount(); i++ ) {

      if ( !m_Classes[i]->IsDatablock() || !m_Classes[i]->IsExport() )
         continue;

      name = m_Classes[i]->GetName() + IDENT_DATABLOCK;
      //if ( m_TempArray.Index( name, false ) ) {
         m_TempArray.Add( name );
      //}
   }

   BuildString( m_TempArray, &m_DatablockList, &m_DatablockIndex );

   //
   // Gather all the concrete class types for 
   // autocomplete when the type name is needed.
   //
   m_TempArray.Empty();
   for ( int i=0; i < m_Classes.GetCount(); i++ ) {

      if ( !m_Classes[i]->IsExport() )
         continue;

      name = m_Classes[i]->GetName() + IDENT_CLASS;
      m_TempArray.Add( name );
   }

   BuildString( m_TempArray, &m_ClassList, &m_ClassIndex );

   //
   // Gather all global general identifiers into the 
   // main autocomplete list...
   //
   // Objects (both types and instances)
   // Datablocks (both types and instances)
   // Namespaces
   // Global functions
   // TorqueScript Keywords
   //
   m_TempArray.Empty();
   for ( int i=0; i < m_Functions.GetCount(); i++ ) {

      name = m_Functions[i]->GetName() + IDENT_FUNCTION;
      //if ( m_TempArray.Index( name, false ) ) {
         m_TempArray.Add( name );
      //}
   }

   for ( int i=0; i < m_Classes.GetCount(); i++ ) {

      name = m_Classes[i]->GetName();
      if ( m_Classes[i]->IsDatablock() )
         name += IDENT_DATABLOCK;
      else
         name += IDENT_CLASS;

      //if ( m_TempArray.Index( name, false ) ) {
      m_TempArray.Add( name );
      //}
   }

   for ( int i=0; i < m_Objects.GetCount(); i++ ) 
   {
      name = m_Objects[i]->GetName();

      // Skip adding unnamed objects.
      if ( name.IsEmpty() )
         continue;

      if ( m_Objects[i]->IsNamespace() )
         name += IDENT_NAMESPACE;
      else if ( m_Objects[i]->IsDatablock() )
         name += ITYPE_DATABLOCKOBJECT;
      //else if ( objects[i]->IsDatablock() )
      //   name += IDENT_NAMESPACE;
      else
         name += IDENT_OBJECT;

      //if ( m_TempArray.Index( name, false ) ) {
         m_TempArray.Add( name );
      //}
   }

   // Add in the reserved words.
   m_TempArray.Add( "break?8" );
   m_TempArray.Add( "case?8" );
   m_TempArray.Add( "continue?8" );
   m_TempArray.Add( "datablock?8" );
   m_TempArray.Add( "default?8" );
   m_TempArray.Add( "else?8" );
   m_TempArray.Add( "false?8" );
   m_TempArray.Add( "for?8" );
   m_TempArray.Add( "function?8" );
   m_TempArray.Add( "if?8" );
   m_TempArray.Add( "local?8" );   
   m_TempArray.Add( "new?8" );
   m_TempArray.Add( "or?8" );
   m_TempArray.Add( "package?8" );
   m_TempArray.Add( "return?8" );
   m_TempArray.Add( "singleton?8" );  
   m_TempArray.Add( "switch?8" );
   m_TempArray.Add( "switch$?8" );
   m_TempArray.Add( "true?8" );
   m_TempArray.Add( "while?8" );

   BuildString( m_TempArray, &m_CompList, &m_CompIndex );
}


AutoCompClass* AutoCompData::FindClassOrObject( const wxString& name ) const
{
   AutoCompClass* found = AutoCompClass::Find( name, m_Classes );
   if ( found )
   {
      // TODO: Sometimes this happens... a user can specifiy a object
      // name that is the same as a class name and vice versa... we 
      // gotta just deal with it!
      //wxASSERT( !AutoCompClass::Find( name, m_Objects ) );
      return found;   
   }

   found = AutoCompClass::Find( name, m_Objects );
   if ( found )
   {
      // TODO: Sometimes this happens... a user can specifiy a object
      // name that is the same as a class name and vice versa... we 
      // gotta just deal with it!
      //wxASSERT( !AutoCompClass::Find( name, m_Classes ) );
      return found;   
   }

   return NULL;
}

void AutoCompData::MergeObjects( const AutoCompClassArray& source )
{
   for ( int i=0; i < source.GetCount(); i++ ) 
   {
      AutoCompClass* object = source[i]; 
      wxASSERT( object );

      const wxString& name = object->GetName();      
      AutoCompClass* found = AutoCompClass::Find( name, m_Objects );

      // Don't merge unnamed objects.
      if ( !found || name.IsEmpty() )
         m_Objects.Add( object );
      else
      {
         // Must copy dupes... as the parent takes ownership
         // and we do not want them to be deleted out from
         // under the page.
         found->AddDupe( new AutoCompClass( *object ) );
      }
   }
}

void AutoCompData::MergeVars( const AutoCompVarArray& source )
{
   wxString file;
   int line;

   for ( int i=0; i < source.GetCount(); i++ ) 
   {
      AutoCompVar* var = source[i];
      wxASSERT( var );

      // We only merge in globals.
      if ( !var->IsGlobal() )
         continue;

      // Look for an existing var.
      AutoCompVar* found = AutoCompVar::Find( var->GetName(), m_Vars );
      if ( !found )
         m_Vars.Add( var );
      else
      {
         // If the found one doesn't have a description
         // then use the new one.
         if ( found->GetDesc().IsEmpty() )
            found->SetDesc( var->GetDesc() );

         // Add the files and lines to the found var.
         for ( int l=0; l < var->GetLines().GetCount(); l++ ) 
         {
            AutoCompVar::GetFileAndLine( var->GetLines()[l], file, line );
            found->AddLine( file, line );
         }
      }
   }
}

void AutoCompData::MergeFunctions( const AutoCompFunctionArray& functions )
{
   // When merging in functions look for namespaced functions
   // and add them to existing datablocks or classes.

   AutoCompFunction* found;
   AutoCompFunction* func;
   wxString nspace;
   AutoCompClass* class_;

   for ( int i=0; i < functions.GetCount(); i++ ) 
   {
      func = functions[i];
      wxASSERT( func );

      nspace = func->GetNamespace();
      if ( !nspace.IsEmpty() ) 
      {
         // Copy the function... we don't want the class to delete
         // the one already owned by the page.
         func = new AutoCompFunction( *func );
         func->SetName( func->GetMethodName() );

         // Find an existing class type or object instance...
         class_ = FindClassOrObject( nspace );
         if ( !class_ ) 
         {
            class_ = new AutoCompClass( nspace, wxEmptyString );
            m_Objects.Add( class_ );
            m_Namespaces.Add( class_ );
         }

         wxASSERT( class_ );

         found = AutoCompFunction::Find( func->GetName(), class_->GetFunctions() );
         if ( !found )
            class_->AddFunction( func );
         else
            found->AddDupe( func );

         continue;
      }

      // It's a plain old function then.
      found = AutoCompFunction::Find( func->GetName(), m_Functions );
      if ( !found )
         m_Functions.Add( func );
      else
      {
         // Must copy dupes... as the parent takes ownership
         // and we do not want them to be deleted out from
         // under the page.
         found->AddDupe( new AutoCompFunction( *func ) );
      }
   }
}

int AutoCompData::BuildMemberList( const wxString& name, wxString& list ) const
{
   // First find the object...
   const AutoCompClass* found = FindClassOrObject( name );
   if ( !found )
      return 0;

   // TODO: Should we add members of dupes?

   // Now loop till we run out of base classes.
   wxArrayString tempArray;
   wxString temp;
   size_t len = 0;
   while ( found )
   {
      for ( int i=0; i < found->GetFunctions().GetCount(); i++ ) 
      {
         temp = found->GetFunctions()[i]->GetMethodName() + IDENT_FUNCTION;
         len += temp.Len() + 1;
         tempArray.Add( temp );
      }

      for ( int i=0; i < found->GetVars().GetCount(); i++ ) 
      {
         temp = found->GetVars()[i]->GetName() + IDENT_VAR;
         len += temp.Len() + 1;
         tempArray.Add( temp );
      }

      const wxString& base = found->GetBase();
      if ( base.IsEmpty() )
         break;

      found = FindClassOrObject( base );
   }

   wxString last, member;
   tempArray.Sort( CmpStringNoCase );
   list.Alloc( len - 1 );
   for ( int i=0; i < tempArray.GetCount(); i++ ) {
      member = tempArray[i];
      if ( member.CmpNoCase( last ) == 0 )
         continue;
      list.Append( member );
      list.Append( ' ' );
      last = member;
   }
   list.RemoveLast();
   list.Shrink();

   return (int)tempArray.GetCount();
}

int AutoCompData::BuildFieldList( const wxString& word, const wxString& path, int line, wxString& list ) const
{
   //wxASSERT( !path.IsEmpty() );
   wxASSERT( line >= 0 );

   // Find the right file...
   AutoCompPage* file = AutoCompPage::Find( path, m_Files );
   if ( !file )
      return 0;

   // TODO: Gotta add unnammed objects as well to make this 
   // work.  Maybe give them a special name... something easy
   // to skip over?  Maybe unnammed objects do not get merged?

   // Find the object that spans the least amount of lines.
   AutoCompClass* object, *found = NULL;
   int span, maxSpan = INT_MAX;
   const AutoCompClassArray& objects = file->GetObjects();
   for ( int i=0; i < objects.GetCount(); i++ )
   {
      object = objects[i];
      wxASSERT( object );

      // Are we within this one?
      span = object->GetLineSpan();
      if ( object->InLineSpan( line ) && span < maxSpan )
      {
         found = object;
         maxSpan = span;
      }
   }

   if ( !found )
      return 0;

   // Loop thru the base classes building a list of fields.
   wxArrayString fields;
   for ( ;; )
   {
      found = AutoCompClass::Find( found->GetBase(), m_Classes );
      if ( !found )
         break;

      found->BuildFieldList( fields );
   }

   // Gotta sort before i can find the word.
   fields.Sort( CmpStringNoCase );

   // If none of the fields start with the word
   // then skip out the string build and return
   // no results.
   if (  !word.IsEmpty() && 
         FindPartialWord( word, fields ) == wxNOT_FOUND )
      return 0;

   // Sort it and make a string out of it!
   BuildString( fields, &list );
   return fields.GetCount();
}

int AutoCompData::BuildLocalsList( const wxString& path, int line, wxString& list ) const
{
   //wxASSERT( !path.IsEmpty() );
   wxASSERT( line >= 0 );

   // Find the right file...
   AutoCompPage* file = AutoCompPage::Find( path, m_Files );
   if ( !file )
      return 0;

   const AutoCompFunctionArray& functions = file->GetFunctions();
   AutoCompFunction* func;
   for ( size_t i=0; i < functions.GetCount(); i++ ) {

      func = functions[i];
      wxASSERT( func );

      // Get the right fuction dupe for this file and line.
      if ( func->InLineSpan( line ) )
         return func->BuildLocalsList( list );
   }

   // TODO: Add check for page locals as well since they
   // are supported.

   return 0;
}


bool AutoCompData::BuildPageFunctions( const wxString& path, wxArrayString& functions, wxArrayInt& startLines, wxArrayInt& endLines ) const
{
   //wxASSERT( !path.IsEmpty() );

   // Find the right file...
   AutoCompPage* file = AutoCompPage::Find( path, m_Files );
   if ( !file )
      return false;

   // First grab all the functions.
   const AutoCompFunctionArray& funcs = file->GetFunctions();
   AutoCompFunction* func;
   for ( size_t i=0; i < funcs.GetCount(); i++ ) {

      func = funcs[i];
      wxASSERT( func );
      functions.Add( func->GetName() );
      startLines.Add( func->GetLine() );
      endLines.Add( func->GetLineLast() );
   }

   return true;
}

void AutoCompData::BuildString( wxArrayString& strings, wxString* output, wxArrayInt* index )
{
   wxASSERT( output );
   wxASSERT( index );

   strings.Sort( CmpStringNoCase );
   output->Alloc( strings.GetCount() * 10 ); // Take a guess at the size.
   index->Alloc( strings.GetCount() );

   for ( int i=0; i < strings.GetCount(); i++ )
   {
      index->Add( (int)output->Length() );
      output->Append( strings[i] );
      output->Append( ' ' );
   }

   output->RemoveLast();
   output->Shrink();
}

void AutoCompData::BuildString( wxArrayString& strings, wxString* output )
{
   wxASSERT( output );

   strings.Sort( CmpStringNoCase );
   output->Alloc( strings.GetCount() * 10 ); // Take a guess at the size.

   for ( int i=0; i < strings.GetCount(); i++ ) 
   {
      output->Append( strings[i] );
      output->Append( ' ' );
   }

   output->RemoveLast();
   output->Shrink();
}

void AutoCompData::GetCallTip( const wxString& name, CallTipInfo& tip ) const
{
   // TODO: Add rejection of reserved words to
   // improve performance.

   // TODO: Support special vars like %this.... we need
   // the file and line here to do this.

   // Don't search for vars if it's not one.
   if ( name[0] == '$' || name[0] == '%' ) 
   {
      AutoCompVar* var = AutoCompVar::Find( name, m_Vars );
      if ( var && !var->GetDesc().IsEmpty() ) 
         tip.SetTip( var->GetName(), var->GetDesc() );

      return;
   }

   wxString object, member;
   if ( name.Find( "::" ) != -1 )
   {
      object = name.BeforeFirst( ':' );
      member = name.AfterFirst( ':' );
      member.Remove( 0, 1 );
   }
   else if ( name.Find( '.' ) != -1 )
   {
      object = name.BeforeFirst( '.' );
      member = name.AfterFirst( '.' );
   }
   else
      object = name;

   // Look for object names.
   AutoCompClass* class_ = AutoCompClass::Find( object, m_Classes );
   if ( class_ ) 
   {
      // Do we have a member to find?
      if ( !member.IsEmpty() ) 
      {
         while ( class_ )
         {
            if ( class_->GetCallTip( member, tip ) )
               break;
            
            class_ = AutoCompClass::Find( class_->GetBase(), m_Classes );         
         }

         return;
      }

      wxString decl;
      if ( class_->IsDatablock() )
         decl << "datablock " << class_->GetName();
      else
         decl << "class " << class_->GetName();

      tip.SetTip( decl, class_->GetDesc() );
      return;
   }

   class_ = AutoCompClass::Find( object, m_Objects );
   if ( class_ ) 
   {
      // Do we have a member to find?
      if ( !member.IsEmpty() ) 
      {
         while ( class_ )
         {
            if ( class_->GetCallTip( member, tip ) )
               break;
            
            class_ = AutoCompClass::Find( class_->GetBase(), m_Classes );         
         }

         return;
      }

      wxString decl;
      if ( class_->IsNamespace() )
         decl << "namespace " << class_->GetName();
      else if ( class_->IsDatablock() )
         decl << "datablock " << class_->GetBase() << "( " << class_->GetName() << " )";
      else
         decl << "object " << class_->GetBase() << "( " << class_->GetName() << " )";

      tip.SetTip( decl, class_->GetDesc() );
      return;
   }

   AutoCompFunction* func = AutoCompFunction::Find( name, m_Functions );
   if ( func ) 
   {
      wxString decl;
      decl << "function " << func->GetName();

      if ( !func->GetArgs().IsEmpty() )
         decl << "( " << func->GetArgs() << " )";
      else
         decl << "()";

      tip.SetTip( decl, func->GetDesc() );
      return;
   }
}

// TODO: Support multiple files and lines returned here for definitions.
bool AutoCompData::GetDefinitionFileAndLine( const wxString& name, wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const
{
   wxASSERT( files );
   wxASSERT( lines );

   // TODO: Fix to work with local vars?... ummm yes... no?

   if ( name[0] == '$' )
   {
      AutoCompVar* var = AutoCompVar::Find( name, m_Vars );
      if ( !var )
         return false;

      return var->GetFileAndLine( files, lines, symbols );
   }

   // TODO: Add rejection of reserved words to
   // improve performance.

   // Look for object names.
   wxString object, member;
   if ( name.Find( "::" ) != -1 )
   {
      object = name.BeforeFirst( ':' );
      member = name.AfterFirst( ':' );
      member.Remove( 0, 1 );
   }
   else if ( name.Find( '.' ) != -1 )
   {
      object = name.BeforeFirst( '.' );
      member = name.AfterFirst( '.' );
   }
   else
      object = name;

   AutoCompClass* class_ = AutoCompClass::Find( object, m_Classes );
   if ( !class_ )
      class_ = AutoCompClass::Find( object, m_Objects );

   if ( class_ ) 
   {
      if ( !member.IsEmpty() ) 
      {
         while ( class_ )
         {
            AutoCompFunction* func = AutoCompFunction::Find( member, class_->GetFunctions() );
            if ( func )
               return func->GetFileAndLine( files, lines, symbols );

            class_ = AutoCompClass::Find( class_->GetBase(), m_Classes );         
         }

         // TODO: Add support for member vars.

         return false;
      }

      return class_->GetFileAndLine( files, lines, symbols );
   }

   AutoCompFunction* func = AutoCompFunction::Find( name, m_Functions );
   if ( func )
      return func->GetFileAndLine( files, lines, symbols );

   return false;
}

wxString AutoCompData::GetFunctionNameAt( const wxString& path, int line ) const
{
   // Find the right file...
   AutoCompPage* file = AutoCompPage::Find( path, m_Files );
   if ( !file )
      return wxEmptyString;

   const AutoCompFunctionArray& functions = file->GetFunctions();
   AutoCompFunction* func;
   for ( size_t i=0; i < functions.GetCount(); i++ ) {

      func = functions[i];
      wxASSERT( func );

      // Get the right fuction dupe for this file and line.
      if ( func->InLineSpan( line ) )
         return func->GetName();
   }

   return wxEmptyString;
}

bool AutoCompData::GetFunctionLineRange( const wxString& path, int line, int& start, int& end ) const
{
   // Find the right file...
   AutoCompPage* file = AutoCompPage::Find( path, m_Files );
   if ( !file )
      return false;

   const AutoCompFunctionArray& functions = file->GetFunctions();
   AutoCompFunction* func;
   for ( size_t i=0; i < functions.GetCount(); i++ ) {

      func = functions[i];
      wxASSERT( func );

      if ( func->InLineSpan( line ) )
      {
         start = func->GetLine();
         end = func->GetLineLast();
         return true;
      }
   }

   return false;
}

int AutoCompData::FindString( const wxChar* word, const wxChar* words, const wxArrayInt& indicies )
{
   // Do a simple binary search using the index array
   // to find the strings in the word list.  It's all 
   // sorted, so it should be super quick.
   size_t i,
      lo = 0,
      hi = indicies.GetCount();
   int res;
   const wxChar* other;

   size_t wordLen = wxStrlen( word );

   while ( lo < hi ) {

      i = (lo + hi) / 2;

      other = words + indicies[i];
      res = wxStrnicmp(word, other, wordLen);
      if ( res < 0 )
         hi = i;
      else if ( res > 0 )
         lo = i + 1;
      else
         return i;
   }

   return -1;
}

int AutoCompData::FindPartialWord( const wxString& word, const wxArrayString& array )
{
   // Do a simple binary search as we know the list
   // is sorted... this should be super quick.
   const size_t len = word.Length();
   size_t i,
      lo = 0,
      hi = array.GetCount();
   int res;

   while ( lo < hi ) {

      i = (lo + hi) / 2;
      wxASSERT( array[i] );

      res = wxStrnicmp( word, array[i], len );
      if ( res < 0 )
         hi = i;
      else if ( res > 0 )
         lo = i + 1;
      else
         return i;
   }

   return wxNOT_FOUND;
}

