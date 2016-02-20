// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompClass.h"

#include "AutoComp.h"
#include "AutoCompExports.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


AutoCompClass::AutoCompClass( const wxString& Name, const wxString& Base )
   :  m_Name( Name ), 
      m_Base( Base ),
      m_Datablock( false ),
      m_Functions( CmpNameNoCase ),
      m_Vars( CmpNameNoCase ),
      m_Dupes( CmpNameNoCase ),
      m_Line( 0 ),
      m_LineLast( 0 )
{
}

AutoCompClass::AutoCompClass( const AutoCompClass& Class )
   :  m_Name( Class.m_Name ), 
      m_Base( Class.m_Base ),
      m_Desc( Class.m_Desc ),
      m_Datablock( Class.m_Datablock ),
      m_Functions( CmpNameNoCase ),
      m_Vars( CmpNameNoCase ),
      m_Dupes( CmpNameNoCase ),
      m_File( Class.m_File ),
      m_Line( Class.m_Line ),
      m_LineLast( Class.m_LineLast )
{
   for ( int i=0; i < Class.m_Functions.GetCount(); i++ ) {
      m_Functions.Add( new AutoCompFunction( *Class.m_Functions[i] ) );
   }
   for ( int i=0; i < Class.m_Vars.GetCount(); i++ ) {
      m_Vars.Add( new AutoCompVar( *Class.m_Vars[i] ) );
   }
   for ( int i=0; i < Class.m_Dupes.GetCount(); i++ ) {
      m_Dupes.Add( new AutoCompClass( *Class.m_Dupes[i] ) );
   }
}

AutoCompClass::~AutoCompClass()
{
   WX_CLEAR_ARRAY( m_Functions );
   WX_CLEAR_ARRAY( m_Vars );
   WX_CLEAR_ARRAY( m_Dupes );
}

AutoCompFunction* AutoCompClass::AddFunction( const wxString& Name )
{
   wxASSERT( !Name.IsEmpty() );

   AutoCompFunction* func = AutoCompFunction::Find( Name, m_Functions );
   if ( !func ) {

      func = new AutoCompFunction( Name );
      m_Functions.Add( func );
   }

   return func;
}

void AutoCompClass::AddFunction( AutoCompFunction* func )
{
   wxASSERT( func );
   m_Functions.Add( func );
}

void AutoCompClass::AddFunctions( AutoCompFunctionArray* functions )
{
   wxASSERT( functions );
   
   for ( int i=0; i < functions->GetCount(); i++ )
   {
      wxASSERT( !AutoCompFunction::Find( (*functions)[i]->GetName(), m_Functions ) ); 
      m_Functions.Add( (*functions)[i] );
   }

   functions->Empty();
}

bool AutoCompClass::DeleteFunction( const wxString& name )
{
   AutoCompFunction* func = AutoCompFunction::Find( name, m_Functions );
   if ( !func )
      return false;

   m_Functions.Remove( func );
   delete func;
   return true;
}

AutoCompVar* AutoCompClass::AddVar( const wxString& Name )
{
   wxASSERT( !Name.IsEmpty() );

   AutoCompVar* var = AutoCompVar::Find( Name, m_Vars );
   if ( !var ) {

      var = new AutoCompVar( Name );
      m_Vars.Add( var );
   }

   return var;
}

void AutoCompClass::AddVars( AutoCompVarArray* vars )
{
   wxASSERT( vars );
   
   for ( int i=0; i < vars->GetCount(); i++ )
   {
      wxASSERT( !AutoCompVar::Find( (*vars)[i]->GetName(), m_Vars ) ); 
      m_Vars.Add( (*vars)[i] );
   }

   vars->Empty();
}

bool AutoCompClass::DeleteVar( const wxString& name )
{
   AutoCompVar* var = AutoCompVar::Find( name, m_Vars );
   if ( !var )
      return false;

   m_Vars.Remove( var );
   delete var;
   return true;
}

/*
int AutoCompClass::BuildMemberList( wxString& List ) const
{
   wxArrayString TempArray;
   TempArray.Alloc( m_Functions.GetCount() + m_Vars.GetCount() );
   wxString Name;

   for ( int i=0; i < m_Functions.GetCount(); i++ ) {
      Name = m_Functions[i]->m_Name + AutoCompData::IDENT_FUNCTION;
      TempArray.Add( Name );
   }

   for ( int i=0; i < m_Vars.GetCount(); i++ ) {
      Name = m_Vars[i]->m_Name + AutoCompData::IDENT_GLOBAL;
      TempArray.Add( Name );
   }

   TempArray.Sort( CmpStringNoCase );
   List.Alloc( TempArray.GetCount() * 10 ); // Take a guess at the size.
   for ( int i=0; i < TempArray.GetCount(); i++ ) {
      List.Append( TempArray[i] );
      List.Append( ' ' );
   }
   List.RemoveLast();
   List.Shrink();

   return TempArray.GetCount();
}
*/

bool AutoCompClass::GetCallTip( const wxString& name, CallTipInfo& tip ) const
{
   AutoCompVar* var = AutoCompVar::Find( name, m_Vars );
   if ( var ) 
   {
      wxString decl;
      decl << m_Name << "::" << var->m_Name;

      tip.SetTip( decl, var->m_Desc );

      return true;
   }

   AutoCompFunction* func = AutoCompFunction::Find( name, m_Functions );
   if ( func ) 
   {
      wxString decl;
      decl << "function " << m_Name << "::" << func->GetMethodName();

      if ( !func->GetArgs().IsEmpty() )
         decl << "( " << func->GetArgs() << " )";
      else
         decl << "()";

      wxString desc = func->GetDesc();

      // Add a note to the user if it's multiply defined.
      //
      // TODO: We should probably return an array of call tips to
      // send all definitions!
      int count = func->GetDupeCount();
      if ( count > 0 )
         desc << "\n" << count << " other definition(s)...";

      tip.SetTip( decl, desc );

      return true;
   }

   return false;
}

/*
void AutoCompClass::Merge( const AutoCompClass* Class )
{
   wxASSERT( Class );

   // We only merge this data if we don't
   // already have a base.
   if ( m_Base.IsEmpty() ) {

      m_Base = Class->m_Base;
      m_Desc = Class->m_Desc;
      m_Page = Class->m_Page;
      m_Line = Class->m_Line;
   }

   // Add new functions or replace existing ones.
   const AutoCompFunctionArray& Functions = Class->m_Functions;
   for ( int f=0; f < Functions.GetCount(); f++ ) {
      
      wxASSERT( Functions[f] );

      AutoCompFunction* func = AutoCompFunction::Find( Functions[f]->m_Name, m_Functions );
      if ( func ) {

         m_Functions.Remove( func );
         delete func;
      }

      func = new AutoCompFunction( *Functions[f] );
      m_Functions.Add( func );
   }

   // Merge in the vars.
   const AutoCompVarArray& Vars = Class->m_Vars;
   for ( int v=0; v < Vars.GetCount(); v++ ) {
      
      wxASSERT( Vars[v] );
      AutoCompVar* Var = AddVar( Vars[v]->m_Name );

      if ( Var->m_Desc.IsEmpty() )
         Var->m_Desc = Vars[v]->m_Desc;

      for ( int l=0; l < Vars[v]->m_Lines.GetCount(); l++ )
          Var->AddLine( Vars[v]->m_Files[l], Vars[v]->m_Lines[l] );
   }

}
*/

void AutoCompClass::AddDupe( AutoCompClass* dupe )
{
   wxASSERT( dupe );
   //wxASSERT( m_Dupes.Index( dupe ) == wxNOT_FOUND ); // This compares using or predicate!
   m_Dupes.Add( dupe );
}

void AutoCompClass::MergeMembers( const AutoCompUnsortedClassArray& objects, const AutoCompExportsArray& exports )
{
   // The objects and exports array should match in 
   // size and be in the same order.
   wxASSERT( objects.GetCount() == exports.GetCount() );

   // We move them into here.
   AutoCompFunctionArray functions( CmpNameNoCase );
   AutoCompVarArray vars( CmpNameNoCase );

   for ( int i=0; i < objects.GetCount(); i++ ) {

      AutoCompClass* object = objects[i];
      if ( !object )
         continue;
      
      while ( !object->m_Functions.IsEmpty() ) {
      
         AutoCompFunction* func = object->m_Functions[0];
         wxASSERT( func && !func->GetName().IsEmpty() );
         object->m_Functions.Remove( func );

         int exportBits = 1 << i;

         // Remove this function from the other objects.
         for ( int j=0; j < objects.GetCount(); j++ ) {
            
            if ( j == i || !objects[j] )
               continue;

            // Note we're making the assumption that a function
            // never changes functionality between different
            // configs (we're not checking args and return types).
            AutoCompFunction* match = AutoCompFunction::Find( func->GetName(), objects[j]->m_Functions );
            wxASSERT( match != func );

            if ( match ) {
               exportBits |= 1 << j;
               objects[j]->m_Functions.Remove( match );

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
         wxASSERT( !func->GetName().IsEmpty() );
         wxASSERT( functions.Index( func ) == -1 );
         functions.Add( func );
      }

      while ( !object->m_Vars.IsEmpty() ) {
      
         AutoCompVar* var = object->m_Vars[0];
         wxASSERT( var && !var->m_Name.IsEmpty() );
         object->m_Vars.Remove( var );

         int exportBits = 1 << i;

         // Remove this far from the other objects.
         for ( int j=0; j < objects.GetCount(); j++ ) {
            
            if ( j == i || !objects[j] )
               continue;

            AutoCompVar* match = AutoCompVar::Find( var->m_Name, objects[j]->m_Vars );
            wxASSERT( match != var );

            if ( match ) {
               exportBits |= 1 << j;
               objects[j]->m_Vars.Remove( match );

               if ( var->GetDesc().IsEmpty() )
                  var->SetDesc( match->GetDesc() );

               delete match;
            }
         }

         // If the object doesn't exist in all pages then
         // we need to add a notation of what configs it
         // does exist in.
         var->m_Desc = AutoCompExports::MakeExistNote( var->m_Desc, exportBits, exports );

         // Add the function to the page.
         wxASSERT( !var->m_Name.IsEmpty() );
         wxASSERT( vars.Index( var ) == -1 );
         vars.Add( var );
      }
   }

   // Replace the functions and vars.
   wxASSERT( m_Functions.IsEmpty() );
   wxASSERT( m_Vars.IsEmpty() );

   m_Functions = functions;
   m_Vars = vars;
}

AutoCompClass* AutoCompClass::Find( const wxString& Name, const AutoCompClassArray& Array )
{
   // Do a simple binary search as we know the list
   // is sorted... this should be super quick.
   size_t i,
      lo = 0,
      hi = Array.GetCount();
   int res;

   while ( lo < hi ) {

      i = (lo + hi) / 2;
      wxASSERT( Array[i] );

      res = Name.CmpNoCase( Array[i]->m_Name );
      if ( res < 0 )
         hi = i;
      else if ( res > 0 )
         lo = i + 1;
      else
         return Array[i];
   }

   return NULL;
}

void AutoCompClass::SetDatablocks( AutoCompClassArray* classes )
{
   wxASSERT( classes );

   wxArrayString datablocks;
   datablocks.Alloc( classes->GetCount() );

   // First we gotta figure out what is a datablock and
   // what is a class.  Do a first pass grabing all the
   // objects names that are or derive from SimDataBlock.
   for ( int i=0; i < classes->GetCount(); i++ ) 
   {
      AutoCompClass* class_ = (*classes)[i];
      wxASSERT( class_ );

      if (  class_->GetName().CmpNoCase( "SimDataBlock" ) == 0 ||
            class_->GetBase().CmpNoCase( "SimDataBlock" ) == 0 )
      {
         datablocks.Add( class_->GetName() );
         class_->SetDatablock( true );
      }
   }

   // Now a second pass... for each class see if it derives
   // from an class in the datablock name array.  If we find 
   // a match we must restart the loop as it could uncover new
   // datablock bases.
   for ( int i=0; i < classes->GetCount(); ) 
   {
      AutoCompClass* class_ = (*classes)[i];
      wxASSERT( class_ );

      if (  !class_->IsDatablock() && 
            datablocks.Index( class_->GetBase(), false ) != wxNOT_FOUND )
      {
         class_->SetDatablock();
         datablocks.Add( class_->GetName() );
         i = 0;
         continue;
      }

      i++;
   }
}

bool AutoCompClass::IsNamespace() const 
{
   return !m_File.IsEmpty() && !HasBase();
}

bool AutoCompClass::IsExport() const
{
   if ( !m_File.IsEmpty() || IsNamespace() )
      return false;

   for ( int i=0; i < m_Dupes.Count(); i++ )
   {
      const AutoCompClass* dupe = m_Dupes[i];
      wxASSERT( dupe );

      if ( !dupe->m_File.IsEmpty() || dupe->IsNamespace() )
         return false;
   }

   return true;
}

bool AutoCompClass::GetFileAndLine( wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const
{
   wxASSERT( files );
   wxASSERT( lines );

   if ( !m_File.IsEmpty() && m_Line > 0 )
   {
      files->Add( m_File );
      lines->Add( m_Line );
      if ( symbols )
         symbols->Add( m_Name );
   }

   // Add function symbols if we have them.
   int first = files->GetCount();
   for ( int i=0; i < m_Functions.GetCount(); i++ )
   {
      const AutoCompFunction* func = m_Functions[i];
      wxASSERT( func );
      func->GetFileAndLine( files, lines, symbols );
   }
   
   // Fixup the symbols to include the class name.
   if ( symbols )
   {
      for ( int i=first; i < symbols->GetCount(); i++ )
         (*symbols)[i] = m_Name + "::" + (*symbols)[i];
   }

   for ( int i=0; i < m_Dupes.Count(); i++ )
   {
      const AutoCompClass* dupe = m_Dupes[i];
      wxASSERT( dupe );

      if ( !dupe->m_File.IsEmpty() && dupe->m_Line > 0 )
      {
         files->Add( dupe->m_File );
         lines->Add( dupe->m_Line );
         if ( symbols )
            symbols->Add( dupe->m_Name );
      }
   }

   wxASSERT( files->GetCount() == lines->GetCount() );
   wxASSERT( files->GetCount() == symbols->GetCount() );
   return !files->IsEmpty();
}

int AutoCompClass::BuildFieldList( wxArrayString& list ) const
{
   for ( int i=0; i < m_Vars.GetCount(); i++ ) 
      list.Add( m_Vars[i]->m_Name + AutoCompData::IDENT_VAR );

   return m_Vars.GetCount();
}
