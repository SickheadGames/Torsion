// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "XmlFile.h"


#include <wx/tokenzr.h>

#ifdef _DEBUG 
#define new DEBUG_NEW 
#endif 

wxChar XmlFile::s_Temp[MAX_PATH];


XmlFile::XmlFile() : XMLDocument()
{
}

XmlFile::XmlFile(const wxChar* buffer)
{
	
	XMLDocument().LoadFile(convertToAscii(buffer));
}

wxString XmlFile::GetStringElem(const wxString& name, const wxString& value)
{
	wxString result = value;
	const char* uuu =name.c_str();

	if (this->FirstChildElement(uuu)) {
		result = this->FirstChildElement(uuu)->GetText();
	}
	//ResetMainPos();
	//FindElem
	//http://www.firstobject.com/dn_markupmethods.htm
	return result;
}

wxPoint XmlFile::GetPointElem(const wxString& name, const wxPoint& value)
{
	return StringToPoint(GetStringElem(name, PointToString(value)));
}

void XmlFile::AddPointElem(const wxString& name, const wxPoint& value)
{


	this->NewElement(name)->SetValue(PointToString(value));

	// AddElem( name.c_str(), PointToString( value ).c_str() );


}

bool XmlFile::GetBoolElem(const wxString& name, bool value)
{
	return StringToBool(GetStringElem(name, value ? "true" : "false"));
}

void XmlFile::AddBoolElem(const wxString& name, bool value)
{
	this->NewElement(name)->SetValue(BoolToString(value));
	//AddElem( name.c_str(), BoolToString( value ).c_str() );
}

int XmlFile::GetIntElem(const wxString& name, int value)
{
	return StringToInt(GetStringElem(name, IntToString(value)));
}

void XmlFile::AddIntElem(const wxString& name, int value)
{
	this->NewElement(name)->SetValue(IntToString(value));
	//  AddElem( name.c_str(), IntToString( value ).c_str() );
}

int XmlFile::GetIntAttrib(const wxString& element, const wxString& attrib, int value)
{


	int result = value;
	
	tinyxml2::XMLElement* elem = this->FirstChildElement(element.c_str());

	if (elem) {

		wxString value(elem->Attribute(attrib.c_str()));
		if (!value.IsEmpty()) {
			result = StringToInt(value);
		}
	}
	//	ResetMainPos();
	return result;
}

wxColour XmlFile::GetColorElem(const wxString& name, const wxColour& value)
{
	return StringToColor(GetStringElem(name, ColorToString(value)));
}

void XmlFile::AddColorElem(const wxString& name, const wxColour& value)
{
	this->NewElement(name)->SetValue(ColorToString(value));

	//AddElem(name.c_str(), ColorToString(value).c_str());
}

wxColour XmlFile::GetColorAttrib(const wxString& element, const wxString& attrib, const wxColour& color)
{
	wxColour result(color);

	tinyxml2::XMLElement* elem = this->FirstChildElement(element.c_str());

	if (elem) {
		wxString value(elem->Attribute(attrib.c_str()));
		if (!value.IsEmpty()) {
			result = StringToColor(value);
		}
	}
	//ResetMainPos();
	return result;
}

wxString XmlFile::ColorToString(const wxColour& color)
{
	wxString Result;
	Result << color.Red() << ", " << color.Green() << ", " << color.Blue();
	return Result;
}

wxColour XmlFile::StringToColor(const wxString& color)
{
	wxStringTokenizer Tokenizer(color, "\t\r\n ,;", wxTOKEN_STRTOK);

	wxString Token = Tokenizer.GetNextToken();
	Token.Trim(true);
	Token.Trim(false);
	long Red = 0;
	Token.ToLong(&Red);
	Red = wxMin(255, wxMax(0, Red));

	Token = Tokenizer.GetNextToken();
	Token.Trim(true);
	Token.Trim(false);
	long Green = 0;
	Token.ToLong(&Green);
	Green = wxMin(255, wxMax(0, Green));

	Token = Tokenizer.GetNextToken();
	Token.Trim(true);
	Token.Trim(false);
	long Blue = 0;
	Token.ToLong(&Blue);
	Blue = wxMin(255, wxMax(0, Blue));

	return wxColour(Red, Green, Blue);
}

wxPoint XmlFile::StringToPoint(const wxString& point)
{
	wxStringTokenizer Tokenizer(point, "\t\r\n ,;", wxTOKEN_STRTOK);

	wxString Token = Tokenizer.GetNextToken();
	Token.Trim(true);
	Token.Trim(false);
	long x = 0;
	Token.ToLong(&x);

	Token = Tokenizer.GetNextToken();
	Token.Trim(true);
	Token.Trim(false);
	long y = 0;
	Token.ToLong(&y);

	return wxPoint(x, y);
}

wxString XmlFile::PointToString(const wxPoint& value)
{
	wxString result;
	result << value.x << " " << value.y;
	return result;
}

char * XmlFile::convertToAscii(const wxChar * str)
{

	char * buffer = new char[sizeof(str)+1];
	int ret;

	ret = wcstombs(buffer, str, sizeof(buffer));
	buffer[sizeof(buffer)] = '\0';
	
	return buffer;
}

int XmlFile::GetArrayStringElems(wxArrayString& output, const wxString& name, const wxString& elemName)
{
	int count = -1;
	
	tinyxml2::XMLElement* elem = this->FirstChildElement(name);

	if (elem) {

		count = 0;

		for (tinyxml2::XMLElement* child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement(elemName)) {

			output.Add(child->GetText());
			++count;
		}
	}
	return count;
}

void XmlFile::AddArrayStringElems(const wxString& name, const wxString& elemName, const wxArrayString& strings)
{
	
	tinyxml2::XMLElement* elem =   this->NewElement(name);

	this->InsertEndChild(elem);
	
	for (size_t i = 0; i < strings.GetCount(); i++) {
		tinyxml2::XMLElement* elem2 =  this->NewElement(elemName);
	
		elem2->SetText(strings[i]);

		elem->InsertEndChild(elem2);
	}
	
}

tinyxml2::XMLElement * XmlFile::AddElem(const wxString & name)
{

	tinyxml2::XMLElement*  elem = this->NewElement(name);

	this->InsertEndChild(elem);

	return elem;
}

tinyxml2::XMLElement * XmlFile::AddElem(const wxString & name, const wxString & value, tinyxml2::XMLElement * element)
{

	tinyxml2::XMLElement*  elem = this->NewElement(name);
	elem->SetText(value),
	element->InsertEndChild(elem);

	return elem;
}

 


