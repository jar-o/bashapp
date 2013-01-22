/*

Copyright (c) 2011 James Robson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


#include "bashapp.h"

int main(int argc, char *argv[]) {

    int i = 0, key_len = 0, icon_len = 0, err = 0, option, retval = 0;
    char *sz = NULL;
    char *script = NULL;
    char *script_e = NULL;
    char *src = NULL;
    char *tmp = NULL, *tmp2 = NULL, *tmp3 = NULL;
    char *key = NULL, *icon = NULL, *key_e = NULL;

	while ((option = getopt(argc, argv, "k:i:")) != -1) {
		switch (option) {
			case 'k':
				key_e = optarg;
				break;
			case 'i':
				icon = optarg;
				break;
			case '?':
				err = 1;
				if (optopt == 'k' || optopt == 'i') {
					printf("Option -%c requires an argument\n", optopt);
				} else {
					printf("Option -%c unrecognized\n", optopt);
				}
				break;
		}
	}

	if (err || argc > 6 || optind >= argc) {
		printf("%s", USAGE);
		return -1;
	}

	if (key == NULL) { // Use default key, recommended, since it's random + variable length
        srand( time(NULL) );
        key_len = (unsigned int)rand() % 
            ( ( (DEFAULT_KEY_LEN * 3) - DEFAULT_KEY_LEN ) + 1 ) + DEFAULT_KEY_LEN;
        key = rand_array(32, 254, key_len);
    } else { // User specified key
        key = calloc(strlen(key_e)+1, sizeof(char)); 
        strncpy(key, key_e, strlen(key_e));
        key_len = strlen(key_e)+1;

	}

    // Load up the script file
    i = load_script(argv[optind++], &script);
	if (i == -1) {
		goto done;
	}

    // Get the script array size in a string formatted number
    sz = calloc( 10, sizeof(char) );
	if (sz == NULL) {
		goto done;
	}
    sprintf(sz, "%d", i);
    
    // Encrypt the script with the given key
    script_e = xor_enc(script, i+1, key, key_len-1);
	if (script_e == NULL) {
		goto done;
	}

    tmp = src_hex_array(script_e, i);
	if (tmp == NULL) {
		goto done;
	}

    // Create code from template
    tmp2 = replace(C_TEMPLATE, "___BASH_SCR___", tmp);
    if (cats(&tmp, NULL) == -1) {
		goto done;
	}

    tmp3 = src_hex_array(key, key_len);
	if (tmp3 == NULL) {
		goto done;
	}

    tmp = replace(tmp2, "___KEY___", tmp3);
    cats(&tmp2, NULL);
    cats(&tmp3, NULL);
	if (tmp == NULL) {
		goto done;
	}

    tmp2 = replace(tmp, "___BASH_SCR_SIZE___", sz);
    cats(&tmp, NULL);
	if (tmp2 == NULL) {
		goto done;
	}

    sprintf(sz, "%d", key_len);
    src = replace(tmp2, "___KEY_LEN___", sz);
    cats(&tmp2, NULL);
	if (src == NULL) {
		goto done;
	}


    // Write the .c source file
    if (cats(&tmp, argv[optind]) == -1) {
		goto done;
	}
    if (cats(&tmp, ".c") == -1) {
		goto done;
	}
	
    retval = write_file(tmp, src);
    cats(&tmp, NULL);
	if (retval == -1) {
		goto done;
	}
    
    // Create and run our 'make' batch script
	tmp = replace(MAKE_APP, "___APPNAME___", argv[optind]);
	if (tmp == NULL) {
		goto done;
	}
    i = system(tmp);
	cats(&tmp, NULL);
	
	// Add the icon if necessary
	if (icon != NULL) {
		tmp = replace(MAKE_ICON, "___ICONFILE___", icon);
		if (tmp == NULL) {
			goto done;
		}
		tmp2 = replace(tmp, "___APPNAME___", argv[optind]);
		if (tmp2 == NULL) {
			goto done;
		}
	}
	i = system(tmp2);
	cats(&tmp2, NULL);

	// Delete .c source file
    if (cats(&tmp2, argv[optind]) == -1) {
		goto done;
	}
    if (cats(&tmp2, ".c") == -1) {
		goto done;
	}
	remove(tmp2);

done:
	if (script != NULL) {
		cats(&script, NULL);
	}
	if (sz != NULL) {
		cats(&sz, NULL);
	}
	if (script_e != NULL) {
		cats(&script_e, NULL);
	}
	if (key != NULL) {
		cats(&key, NULL);
	}
	if (tmp != NULL) {
		cats(&tmp, NULL);
	}
	if (tmp2 != NULL) {
		cats(&tmp2, NULL);
	}
	if (src != NULL) {
		cats(&src, NULL);
	}
    return 0;
}
