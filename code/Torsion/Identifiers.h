// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.


// NOTE:  This is read by DialogBlocks.  It's kinda
// dumb at times, so do not include any #defines other
// than for identifiers... not even a header guard.


// Used for the status bar indicators.
#define tsSTATUS_MAIN      0
#define tsSTATUS_LINE      1
#define tsSTATUS_COL       2
#define tsSTATUS_OFFSET    3
#define tsSTATUS_CHAR      4
#define tsSTATUS_OVERWRITE 5


#define tsID_GOTO                      6000 // wxID_HIGHEST+1
#define tsID_STATUSBAR                 6001
#define tsID_TABSANDSPACES             6002
#define tsID_LINEBREAKS                6003
#define tsID_LINENUMBERS               6004
#define tsID_LINEWRAP                  6005
#define tsID_NEWPROJECT                6006
#define tsID_OPENPROJECT               6007
#define tsID_CLOSEPROJECT              6008
#define tsID_FINDINFILES               6009
#define tsID_SAVEALL                   6010
#define tsID_NEXTVIEW                  6011
#define tsID_PREVIOUSVIEW              6012
#define tsID_CLEARDSO                  6013
#define tsID_CLEARDSOS                 6014
#define tsID_CLOSE_OTHERS              6015
#define tsID_SHOW_LEFTPANE             6016
#define tsID_SHOW_BOTTOMPANE           6017
#define tsID_COPY_FULL_PATH            6018
#define tsID_OPEN_CONTAINING_FOLDER    6019


#define tsID_PROJECT_PROPERTIES        6020
#define tsID_PROJECT_REBUILDEXPORTS    6021
#define tsID_PROJECT_REFRESH           6022
#define tsID_PROJECT_PRECOMPILE        6023
#define tsID_PROJECT_PRECOMPILE_ACTIVE 6024
#define tsID_PROJECT_PRECOMPILEALL     6025
#define tsID_PROJECT_SYNCTREE          6026
#define tsID_PROJECT_SHOWALLMODS       6027
#define tsID_PROJECT_SHOWALLFILES      6028

// Used for the project MRU.
#define tsID_PROJECT1   6030
#define tsID_PROJECT2   6031
#define tsID_PROJECT3   6032
#define tsID_PROJECT4   6033
#define tsID_PROJECT5   6034


// These must be contiguous in this order.
#define tsID_PROJECTSASH   6040
#define tsID_BOTTOMSASH    6041


// These must be contiguous in this order.
#define tsID_DEBUG_START                     6050
#define tsID_DEBUG_START_WITHOUT_DEBUGGING   6051
#define tsID_DEBUG_CONNECT                   6052
#define tsID_DEBUG_STOP                      6053
#define tsID_DEBUG_BREAK                     6054
#define tsID_DEBUG_STARTBREAK                6055
#define tsID_DEBUG_STEP                      6056
#define tsID_DEBUG_STEP_OVER                 6057
#define tsID_DEBUG_STEP_OUT                  6058
#define tsID_DEBUG_RESTART                   6059

// Gotta keep contiguous!
#define tsID_ADVANCED_FIRST            6060
   #define tsID_COMMENT_SELECTION         6061
   #define tsID_UNCOMMENT_SELECTION       6062
   #define tsID_INDENT_SELECTION          6063
   #define tsID_UNINDENT_SELECTION        6064
   #define tsID_UPPER_SELECTION           6065
   #define tsID_LOWER_SELECTION           6066
   #define tsID_MATCH_BRACE               6067
#define tsID_ADVANCED_LAST             6068



// These must be contiguous and in this order.
#define tsID_DEBUG_NEWBREAKPOINT             6070
#define tsID_DEBUG_CLEARALLBREAKPOINTS       6071
#define tsID_DEBUG_DISABLEALLBREAKPOINTS     6072
#define tsID_BREAKPOINT_PROPERTIES           6073
#define tsID_BREAKPOINT_GOTOSOURCE           6075

#define tsID_STOPPED_EVENT                   6076
#define tsID_SOCKET_EVENT                    6077

// Used by the OutputPanel and OutputCtrl.
#define tsID_OUTPUT_GOTOFILE     6080
#define tsID_OUTPUT_ERRORS       6081
#define tsID_OUTPUT_NEXTERROR    6082
#define tsID_OUTPUT_PREVERROR    6083
#define tsID_OUTPUT_CLEAR        6084
#define tsID_OUTPUT_SAVE         6085
#define tsID_OUTPUT_LOAD         6086


// Used mainly by the ScriptCtrl.
#define tsID_SCRIPT_OPENEXEC           6090
#define tsID_SCRIPT_REMOVEBREAKPOINT   6091
#define tsID_SCRIPT_TOGGLEBREAKPOINT   6092
#define tsID_SCRIPT_INSERTBREAKPOINT   6093
#define tsID_SCRIPT_OPENDEF            6094


// Gotta keep contiguous!
#define tsID_FOLDING_FIRST             6100
   #define tsID_FOLDING_TOGGLEBLOCK       6101
   #define tsID_FOLDING_TOGGLEALLINBLOCK  6102
   #define tsID_FOLDING_TOGGLEALL         6103
   #define tsID_FOLDING_COLLAPSETODEFS    6104
#define tsID_FOLDING_LAST              6105


#define tsID_CALLSTACK     6121
#define tsID_FINDNEXT      6122
#define tsID_FINDPREV      6123
#define tsID_FINDCOMBO     6124
#define tsID_CONFIGCOMBO   6125

// Used for Find output window.
#define tsID_FINDWINDOW    6130
#define tsID_CLEARALL      6132
#define tsID_GOTOFILE      6133
#define tsID_FINDCANCEL    6134
#define tsID_FINDSYMBOL    6135


// The following ids are mainly used by 
// the project objects to deal with the 
// file view context menu.
#define tsID_PROJECT_OPEN        6140
#define tsID_PROJECT_OPENWITH    6141
#define tsID_EXPLORE             6142
#define tsID_COPY                6143
#define tsID_MOVE                6144
#define tsID_RENAME              6145
#define tsID_NEW_FOLDER          6146
#define tsID_NEW_FILE            6147

// These must be contiguous.
#define tsID_TOOLS_FIRST            6160
#define tsID_TOOLS_LAST             6180

#define tsID_TOOLS_EXTERNAL_TOOLS   6181
#define tsID_TOOLS_PREFERENCES      6182

#define tsID_UPDATECHECK         6190
#define tsID_BUGTRACKER          6191
#define tsID_FEATUREREQUEST      6192
#define tsID_REGISTER            6193

#define tsID_RELOAD_SCRIPT       6200
#define tsID_RELOAD_SCRIPTS      6201
#define tsID_MOVE_INSTPTR        6202
#define tsID_ADD_WATCH           6203


// These must be contiguous.
#define tsID_SCRIPTSENSE_FIRST   6220
   #define tsID_LIST_MEMBERS        6221
   #define tsID_PARAMETER_INFO      6222
   #define tsID_QUICK_INFO          6223
   #define tsID_COMPLETE_WORD       6224
#define tsID_SCRIPTSENSE_LAST    6225

// These must be contiguous.
#define tsID_BOOKMARK_FIRST      6230
   #define tsID_BOOKMARK_TOGGLE     6231
   #define tsID_BOOKMARK_NEXT       6232
   #define tsID_BOOKMARK_PREV       6233
   #define tsID_BOOKMARK_CLEARALL   6234
#define tsID_BOOKMARK_LAST       6235

// For list of open windows.
#define tsID_WINDOW_FIRST  6240
#define tsID_WINDOW_LAST   6249

// This range is reserved for os specific context menu items.
#define tsID_SHELL_MIN   10000
#define tsID_SHELL_MAX     20000
