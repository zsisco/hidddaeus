/* 
 * hidddaeus is written by Zachary Sisco (2016), and it uses code adapted from 
 * HID Listen and Raw HID I/O Routines by PJRC.COM, LLC and is released under 
 * the GNU General Public License version 3. 
 */

/* HID Listen, http://www.pjrc.com/teensy/hid_listen.html
 * Listens (and prints) all communication received from a USB HID device,
 * which is useful for view debug messages from the Teensy USB Board.
 * Copyright 2008, PJRC.COM, LLC
 *
 * You may redistribute this program and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include "rawhid.h"
#include "hidddaeus.h"

static void delay_ms(unsigned int msec);
static FILE *newsession(char *sessionfile, int num);

int main(void)
{

	char buf[1024], *in, *out;
	rawhid_t *hid;
	int num, count;
	pid_t pid;

	/****************************************************************
     * Daemon start up code adapted from Devin Watson, 
     * written May 2004. Last accessed 10 December 2016.
     * http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
	 ****************************************************************/
	/* Fork off the parent process */
	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	/* If success, then exit the parent process */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* Create a new SID for the child process */
	if (setsid() < 0)
		exit(EXIT_FAILURE);

	/* Catch, ignore and handle signals */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* Change the file mode mask */
	umask(0);

	/* Change the working directory */
	chdir("~/.hidddaeus/");

	/* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
	/****************************************************************/

	/* Open the daemon log file */
	openlog("hidddaeus", LOG_PID, LOG_DAEMON);

	/* Working file for storing raw HID input */
	char sessionfile[SESSION_NAME_LEN];
	/* Working file number incrementer */
	int logno = 0;

	syslog(LOG_NOTICE, "hidddaeus started.");
	while (1) {

		/* Open working file for storing raw HID input */
		FILE *hidinput = newsession(&sessionfile, logno);

		hid = rawhid_open_only1(0, 0, 0xFF31, 0x0074);
		if (hid == NULL) {
			fflush(hidinput);
			delay_ms(1000);
			continue;
		}

		syslog(LOG_INFO, "HID connected, listening...");
		while (1) {
			num = rawhid_read(hid, buf, sizeof(buf), 200);
			if (num < 0) break;
			if (num == 0) continue;
			in = out = buf;
			for (count = 0; count < num; count++) {
				if (*in) {
					*out++ = *in;
				}
				in++;
			}
			count = out - buf;
			if (count) {
				num = fwrite(buf, 1, count, hidinput);
				fwrite(" ; ", 1, 3, hidinput);
				syslog(LOG_INFO, buf);
				fflush(hidinput);
				
				/* buf contains exit command, end of session */
				if (strstr(buf, "exit") != NULL) {
					syslog(LOG_INFO, "End of session, %s", sessionfile);

					if (REGEX == 1) {
					/* Send session file to pattern recognition for each attack model */
					int i;
					    for (i = 0; sizeof(attack_models) / sizeof(attack_models[0]); i++) {
						/* attack model name */
						syslog(LOG_INFO, "Attack model: %s", attack_models[i].name);

						/* calculate size of command (cat + session file + grep command pattern) */
						int cmdsize = 18 + SESSION_NAME_LEN + strlen(attack_models[i].pattern);
						syslog(LOG_INFO, "Command size: %d", cmdsize);

						/* allocate dynamic string for command */
						char *cmd;
						cmd = (char *)malloc(cmdsize * sizeof(char));
						snprintf(cmd, cmdsize, "cat %s | grep -c -P \"%s\"", sessionfile, attack_models[i].pattern);
						syslog(LOG_INFO, "Command string: %s", cmd);

						/* exec cat | grep */
						FILE *pipe;
						char result[1024];

						pipe = popen(cmd, "r");
						if (pipe == NULL) {
							perror("popen");
							exit(EXIT_FAILURE);
						}
						while (fgets(result, sizeof(result), pipe)) {
							syslog(LOG_INFO, "Match: %s", result);
						}
						pclose(pipe);
						free(cmd);
					    }
					}
					else if (KNN == 1) {
						FILE *pipe;
						char result[1024];
						char *cmd;
						int cmdsize = 16 + strlen(KNN_FILE) + strlen(USER_FILE) + strlen(sessionfile);
						cmd = (char *)malloc(cmdsize * sizeof(char));
						snprintf(cmd, cmdsize, "python %s %s %s", KNN_FILE, USER_FILE, sessionfile);

						pipe = popen(cmd, "r");
						if (pipe == NULL) {
							perror("popen");
							exit(EXIT_FAILURE);
						}
						while (fgets(result, sizeof(result), pipe)) {
							syslog(LOG_INFO, "%s", result);
						}
						pclose(pipe);
						free(cmd);

					}
					else {
						syslog(LOG_INFO, "No valid configuration.");
					}
					fclose(hidinput);
					logno += 1;
					hidinput = newsession(&sessionfile, logno);
				}
			}
		}
		rawhid_close(hid);
		syslog(LOG_INFO, "HID disconnected, waiting for new device...");

		/* close work session file */
		if (hidinput != NULL)
			fclose(hidinput);
		logno += 1;
	}

	syslog(LOG_NOTICE, "hidddaeus terminated.");
	closelog();
	return 0;
}

static void delay_ms(unsigned int msec)
{
	usleep(msec * 1000);
}

static FILE *newsession(char *sessionfile, int num)
{
	snprintf(sessionfile, SESSION_NAME_LEN, "%s%d", SESSION_NAME, num);
	FILE *hidinput = fopen(sessionfile, "wb");
	if (hidinput == NULL) {
		syslog(LOG_NOTICE, "\nFLAGRANT SYSTEM ERROR: %s cannot be written. Aborting.\n\n", sessionfile); 
		exit(EXIT_FAILURE);
	}

	return hidinput;
}


