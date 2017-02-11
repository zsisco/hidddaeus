/* 
 * hidddaeus is written by Zachary Sisco (2016), and it uses code adapted from 
 * HID Listen and Raw HID I/O Routines by PJRC.COM, LLC and is released under 
 * the GNU General Public License version 3. 
 */

static const unsigned int REGEX = 0;
static const unsigned int   KNN = 1;
static const char    KNN_FILE[] = "kNN.py";
static const char   USER_FILE[] = "user_data";

static const char  SESSION_NAME[] = "hidsession.log.";
static const int SESSION_NAME_LEN = 20;

typedef struct {
	const char *name;
	const char *pattern;
} Attack;

static const Attack attack_models[] = {
	/* name                       pattern         */
	{ "Download exec script",    "(wget|curl).+?;.+?(xxd)?.*?(chmod).+?(&)" },
	{ "Test",                    "(nmap).+?;.+?(xxd)?.*?(chmod).+?(&)"      },
};

