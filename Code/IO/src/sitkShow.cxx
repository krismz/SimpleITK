/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#include "sitkShow.h"
#include "sitkMacro.h"
#include "sitkImageFileWriter.h"
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <ctype.h>
#ifdef WIN32
#include <process.h>
#endif

namespace itk
{
  namespace simple
  {

  static int ShowImageCount = 0;

  // time to wait in mili-seconds before we check if the process is OK
  const unsigned int ProcessDelay = 500;


#if defined(WIN32)
  static std::string ShowImageCommand = "%a -o %f";
  static std::string ShowColorImageCommand = "%a -eval %m";

#elif defined(__APPLE__)
  static std::string ShowImageCommand = "open -a %a %f";
  // the "-n" flag tells OSX to launch a new instance of ImageJ, even if one is already running.
  // we do this because otherwise the macro command line argument is not correctly passed to
  // a previously running instance of ImageJ.
  static std::string ShowColorImageCommand = "open -a %a -n --args -eval %m";

#else
  // linux and other systems
  static std::string ShowImageCommand = "%a -o %f";
  static std::string ShowColorImageCommand = "%a -e %m";
#endif


  // Function to replace %tokens in a string.  The tokens are %a, %f, %m for
  // application, filename and macro respectively.  %% will send % to the output string.
  //
  static std::string ReplaceWords(std::string command, std::string app, std::string filename, std::string macro, bool& fileFlag)
    {
    std::string result;

    unsigned int i=0;
    while (i<command.length())
      {

      if (command[i] != '%')
        {
        result.push_back(command[i]);
        }
      else
        {
        if (i<command.length()-1)
          {
          // check the next character after the %
          switch(command[i+1])
            {
            case '%':
              // %%
              result.push_back(command[i]);
              i++;
              break;
            case 'a':
              // %a for application
              result.append(app);
              i++;
              break;
            case 'f':
              // %f for filename
              result.append(filename);
              fileFlag = true;
              i++;
              break;
            case 'm':
              // %m for macro
              result.append(macro);
              fileFlag = true;
              i++;
              break;
            }

          }
        else
          {
          // if the % is the last character in the string just pass it through
          result.push_back(command[i]);
          }
        }
      i++;
      }
    return result;
    }

  // Function to strip the matching leading and trailing quotes off a string
  // if there are any.  We need to do this because the way arguments are passed
  // to itksysProcess_Execute
  //
  static std::string UnquoteWord(std::string word)
    {
    size_t l = word.length();

    if (l<2)
      {
      return word;
      }
    //std::cout << "crap: " << word[0] << " " << word[l-1] << " " << l << std::endl;

    switch(word[0])
      {

      case '\'':
      case '\"':
        if (word[l-1] == word[0])
          {
          std::cout <<  "Unquoted: " << word.substr(1, l-2);
          return word.substr(1, l-2);
          }
        else
          {
          return word;
          }
        break;

      default:
        return word;
        break;
      }
    }

  static std::vector<std::string> ConvertCommand(std::string command, std::string app, std::string filename, std::string macro="")
    {
    bool fileFlag=false;
    std::string new_command = ReplaceWords(command, app, filename, macro, fileFlag);
    std::istringstream iss(new_command);
    std::vector<std::string> result;
    std::vector<unsigned char> quoteStack;
    std::string word;
    unsigned int i=0;

    //std::cout << new_command << std::endl;

    while (i<new_command.length())
      {
      switch (new_command[i])
        {

        case '\'':
        case '\"':
          word.push_back(new_command[i]);
          if (quoteStack.size())
            {
            if (new_command[i] == quoteStack[quoteStack.size()-1])
              {
              // we have a matching pair, so pop it off the stack
              quoteStack.pop_back();
              }
            else
              {
              // the top of the stack and the new one don't match, so push it on
              quoteStack.push_back(new_command[i]);
              }
            }
          else
            {
            // quoteStack is empty so push this new quote on the stack.
            quoteStack.push_back(new_command[i]);
            }
          break;

        case ' ':
          if (quoteStack.size())
            {
            // the space occurs inside a quote, so tack it onto the current word.
            word.push_back(new_command[i]);
            }
          else
            {
            // the space isn't inside a quote, so we've ended a word.
            word = UnquoteWord(word);
            result.push_back(word);
            word.clear();
            }
          break;

        default:
          word.push_back(new_command[i]);
          break;
        }
      i++;

      }

    if (word.length())
      {
      word = UnquoteWord(word);
      result.push_back(word);
      }


    // if the filename token wasn't found in the command string, add the filename to the end of the command vector.
    if (!fileFlag)
      {
      result.push_back(filename);
      }
    return result;
    }

  //
  static std::string FormatFileName ( std::string TempDirectory, std::string name )
  {
  std::string TempFile = TempDirectory;
  std::string Extension = ".nii";

  itksys::SystemTools::GetEnv ( "SITK_SHOW_EXTENSION", Extension );

  if ( name != "" )
    {
    TempFile = TempFile + name + Extension;
    }
  else
    {
    std::ostringstream tmp;

#ifdef WIN32
    int pid = _getpid();
#else
    int pid = getpid();
#endif

    tmp << "TempFile-" << pid << "-" << ShowImageCount;
    ShowImageCount++;
    TempFile = TempFile + tmp.str() + Extension;
    }
  return TempFile;
  }

#if defined(WIN32)
  //
  // Function that converts slashes or backslashes to double backslashes.  We need
  // to do this so the file name is properly parsed by ImageJ if it's used in a macro.
  //
  static std::string DoubleBackslashes(const std::string word)
  {
    std::string result;

    for (unsigned int i=0; i<word.length(); i++)
      {
      // put in and extra backslash
      if (word[i] == '\\' || word[i]=='/')
        {
        result.push_back('\\');
        result.push_back('\\');
        }
      else
        {
        result.push_back(word[i]);
        }
      }

    return result;
  }
#endif

  //
  //
  static std::string BuildFullFileName(const std::string name)
  {
  std::string TempDirectory;

#ifdef WIN32
  if ( !itksys::SystemTools::GetEnv ( "TMP", TempDirectory )
    && !itksys::SystemTools::GetEnv ( "TEMP", TempDirectory )
    && !itksys::SystemTools::GetEnv ( "USERPROFILE", TempDirectory )
    && !itksys::SystemTools::GetEnv ( "WINDIR", TempDirectory ) )
    {
    sitkExceptionMacro ( << "Can not find temporary directory.  Tried TMP, TEMP, USERPROFILE, and WINDIR environment variables" );
    }
  TempDirectory = TempDirectory + "\\";
  TempDirectory = DoubleBackslashes(TempDirectory);
#else
  TempDirectory = "/tmp/";
#endif
  return FormatFileName ( TempDirectory, name );
  }


  //
  //
  static std::string FindApplication(const std::string name)
  {

  std::vector<std::string> paths;
  std::string ExecutableName=name;

#ifdef WIN32
  std::string ProgramFiles;
  if ( itksys::SystemTools::GetEnv ( "PROGRAMFILES", ProgramFiles ) )
    {
    paths.push_back ( ProgramFiles + "\\ImageJ\\" );
    }

    // Find the executable
    ExecutableName = itksys::SystemTools::FindFile ( ExecutableName.c_str(), paths );
    if ( ExecutableName == "" )
      {
      sitkExceptionMacro ( << "Can not find location of " << name );
      }

#elif defined(__APPLE__)

  paths.push_back("/Applications"); //A common place to look
  paths.push_back("/Developer"); //A common place to look
  paths.push_back("/opt/ImageJ");   //A common place to look
  paths.push_back("/usr/local/ImageJ");   //A common place to look

  ExecutableName = itksys::SystemTools::FindDirectory( name.c_str() );

#else

  // linux and other systems
  ExecutableName = itksys::SystemTools::FindFile ( name.c_str() );

#endif

  return ExecutableName;
  }


  /**
   * This function take a list of command line arguments, and runs a
   * process based on it. It waits for a fraction of a second before
   * checking it's state, to verify it was launched OK.
   */
  static void ExecuteShow( const std::vector<std::string> & cmdLine )
  {

#ifndef NDEBUG
    std::copy( cmdLine.begin(), cmdLine.end(), std::ostream_iterator<std::string>( std::cout, "\n" ) );
    std::cout << std::endl;
#endif

    std::vector<const char*> cmd( cmdLine.size() + 1, NULL );

    for ( unsigned int i = 0; i < cmdLine.size(); ++i )
      {
      cmd[i] = cmdLine[i].c_str();
      }

    itksysProcess *kp = itksysProcess_New();

    itksysProcess_SetCommand( kp, &cmd[0] );

    itksysProcess_SetOption( kp, itksysProcess_Option_Detach, 1 );

    // For detached processes, this appears not to be the default
    itksysProcess_SetPipeShared( kp, itksysProcess_Pipe_STDERR, 1);
    itksysProcess_SetPipeShared( kp, itksysProcess_Pipe_STDOUT, 1);

    itksysProcess_Execute( kp );

    // Wait one second then check to see if we launched ok.
    // N.B. Because the launched process may spawn a child process for
    // the acutal application we want, this methods is needed to be
    // called before the GetState, so that we get more then the
    // imediate result of the initial execution.
    double timeout = ProcessDelay;
    itksysProcess_WaitForExit( kp, &timeout );


    switch (itksysProcess_GetState(kp))
      {
      case itksysProcess_State_Executing:
        // This first case is what we expect if everything went OK.

        // We want the other process to continue going
        itksysProcess_Delete( kp ); // implicitly disowns
        break;

      case itksysProcess_State_Exited:
        {
        int exitValue = itksysProcess_GetExitValue(kp);
        if ( exitValue != 0 )
          {
          sitkExceptionMacro (  << "Process returned " << exitValue << "." );
          }
        }
        break;

      case itksysProcess_State_Killed:
        itksysProcess_Delete( kp );
        sitkExceptionMacro (  << "Child was killed by parent." );
        break;

      case itksysProcess_State_Exception:
        {
        std::string exceptionString = itksysProcess_GetExceptionString(kp);
        itksysProcess_Delete( kp );
        sitkExceptionMacro (  << "Child terminated abnormally: " << exceptionString );;
        }
        break;

      case itksysProcess_State_Error:
        {
        std::string errorString = itksysProcess_GetErrorString(kp);
        itksysProcess_Delete( kp );
        sitkExceptionMacro (  << "Error in administrating child process: [" << errorString << "]" );
        }
        break;

      // these states should not occour, because they are the result
      // from actions we don't take
      case itksysProcess_State_Expired:
      case itksysProcess_State_Disowned:
      case itksysProcess_State_Starting:
      default:
        itksysProcess_Delete( kp );
        sitkExceptionMacro (  << "Unexpected process state!" );
      };

  }

  void Show( const Image &image, const std::string name)
  {
  // Try to find ImageJ, write out a file and open
  std::string ExecutableName;
  std::string Command, Command3D;
  std::string TempFile = "";
  std::string Macro = "";
  std::vector<std::string> CommandLine;


  TempFile = BuildFullFileName(name);
  //std::cout << "Full file name:\t" << TempFile << std::endl;

  // write out the image
  WriteImage ( image, TempFile );


  bool colorFlag = false;


  // If the image is 3 channel, 8 or 16 bit, assume it's a color image and build an
  // ImageJ macro to view it as a composite image.
  //
  if ( (image.GetNumberOfComponentsPerPixel() == 3)
    && ((image.GetPixelIDValue()==sitkVectorUInt8) || (image.GetPixelIDValue()==sitkVectorUInt16)) )
    {
#if defined(WIN32)
    // Windows needs single quotes while *nix needs double.
    Macro = "\"open(\'" + TempFile + "\');";
    Macro += " run(\'Make Composite\', \'display=Composite\'); \"";
#else
    Macro = "\'open(\"" + TempFile + "\");";
    Macro += " run(\"Make Composite\", \"display=Composite\"); \'";
#endif
    colorFlag = true;
    //std::cout << "Macro:\t" << Macro << std::endl;
    }



  // check for user-defined environment variables
  //
  if (colorFlag)
    {
    itksys::SystemTools::GetEnv ( "SITK_SHOW_COLOR_COMMAND", Command );
    if (!Command.length())
      {
      itksys::SystemTools::GetEnv ( "SITK_SHOW_COMMAND", Command );
      }
    if (!Command.length())
      {
      Command = ShowColorImageCommand;
      }
    }
  else
    {
      itksys::SystemTools::GetEnv ( "SITK_SHOW_COMMAND", Command );
      if (!Command.length())
        {
        Command = ShowImageCommand;
        }
    }
  itksys::SystemTools::GetEnv ( "SITK_SHOW_3D_COMMAND", Command3D );
  if (!Command3D.length())
    {
    Command3D = Command;
    }

  if (image.GetDimension() == 3)
    {
    Command = Command3D;
    }


#if defined(WIN32)

  // Windows
  //
  ExecutableName = FindApplication("ImageJ.exe");

  CommandLine = ConvertCommand(Command, ExecutableName, TempFile, Macro);

#elif defined(__APPLE__)

# if defined(__x86_64__)
  // Mac 64-bit
  //
  // This is a funny special case because it tries to launch ImageJ64, then if that
  // fails it falls through to the Mac 32-bit code.
  //
  ExecutableName = FindApplication( "ImageJ64.app" );
  if( ExecutableName == "" )
    {
    // Just assume it is registered properly in a place where the open command will find it.
    ExecutableName="ImageJ64";
    }

  CommandLine = ConvertCommand(Command, ExecutableName, TempFile, Macro);

  bool fail64 = false;

  try
    {
    ExecuteShow( CommandLine );
    }
  catch(...)
    {
    fail64 = true;
    }

  if (!fail64)
    // 64-bit app worked, so we're done
    return;

   // if 64-bit app failed, we fall through to the 32-bit code
#  endif // __x86_64__


  // Mac 32-bit
  ExecutableName = FindApplication( "ImageJ.app" );

  CommandLine = ConvertCommand(Command, ExecutableName, TempFile, Macro);

#else

  // Linux and other systems
  ExecutableName = FindApplication("ImageJ");
  if (ExecutableName == "")
    {
    ExecutableName = FindApplication("imagej");
    }

  CommandLine = ConvertCommand(Command, ExecutableName, TempFile, Macro);
#endif

  // run the compiled command-line in a process which will detach
  ExecuteShow( CommandLine );
  }

  }
}
