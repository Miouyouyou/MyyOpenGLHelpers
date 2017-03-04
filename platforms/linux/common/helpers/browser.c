#include <string.h> // memcpy
#include <stdlib.h> // system

#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>

const char browser_command_name[] = "xdg-open ";
#define SIZEOF_BROWSER_COMMAND (sizeof(browser_command_name)-1)

/** Browse the provided url using xdg-open to launch the appropriate
 *  browser
 *
 *  @param url The URL of the document to browse
 */
void myy_open_website(const char * restrict const url) {

  unsigned int url_size = myy_string_size((uint8_t * const) url);
  // Let's avoid blowing up the stack with an URL
  url_size = url_size < 512 ? url_size : 512;

  unsigned int browser_command_full_size =
    SIZEOF_BROWSER_COMMAND + url_size;
  uint8_t browser_command[browser_command_full_size+1];

  memcpy(browser_command, browser_command_name, SIZEOF_BROWSER_COMMAND);
  memcpy(browser_command+SIZEOF_BROWSER_COMMAND, url, url_size);

  browser_command[browser_command_full_size] = '\0';

  LOG("[X11/myy_open_website]  \n");
  LOG("  URL : %s, Command : %s\n", url, browser_command);

  system(browser_command);

}
