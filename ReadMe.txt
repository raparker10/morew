MOREW.EXE - "More" for Windows

This project began as an effort to prrovide "more"-like functionality for 
Windows, but the need for quickly examining large (64-bit length) files 
quickly pushed the effort into a graphical UI with easy navigation, 
variable screen width.

To use the program, select File | Open, select a file, and its contents
will be displayed.  Use standard windows program interactions to scroll 
around the file.

Commands
File | Open
View | Go to Top
View | Line Down
View | Line Up
View | Page Down
View | Page Up
View | Narrower
View | Wider
Help | About


CApplication.cpp / .h
A generic application class.  Used as the basis for CFileViewerApp.cpp / .h

CCanvas.cpp / .h
A canvas object used for double-buffering the screen.

CFileBuffer.cpp / .h
Manages file handling operations.

CDrawingHelper.cpp / .h
Implements functionality for shared drawing information and functions.

CFileViewerApp.cpp / .h
The application class for this program.  Derived from CApplication

CMoreScreenLayout.cpp / .h
Stores information for the screen layout.  Derived from the generic CScreenLayout.

CScreenLayout.cpp / .h
A generic class for holding screen layout information.

morew.cpp / .h
Application entry point.  Initializes main application object and window.

Resource.h
Contains values for resource identifiers.

stdafx.cpp / .h
Microsoft Visual Studio pre-compiled header suppport.

To Do
-------------------------------------------------------------------------------
* implement page-up and page-down from the menu
* implement a goto location function
* draw a better icon
* accept a file on the command line
* accept a file via drag-and drop
* integrate with the shell to appear on the context menu
* display the file name on the window frame
* respond to mouse moves by displaying the position under the pointer
* respond to mouse moves by hilighting the hex and character on screen
* enable decoding as Unicode
* enable displaying text files with proper pagination
* create a search by character function
* create a search by text function

Known Bugs
-------------------------------------------------------------------------------
* page-up and page-down move more than one page
* opening a new file does not reset the scrollbar to the top of page
