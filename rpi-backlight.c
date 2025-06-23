/**
 *  Simple program for controlling backlight of the official
 *  Raspberry Pi Touchscreen.
 *
 *  Make sure you execute following commands to be able to access config files as
 *  a regular user:
 *
 *  sudo su -c 'echo SUBSYSTEM==\"backlight\", RUN+=\"/bin/chmod 0666 /sys/class/backlight/%k/brightness /sys/class/backlight/%k/bl_power\" > /etc/udev/rules.d/99-backlight.rules'
 *  sudo reboot
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define POWER_FILE "/sys/class/backlight/backlight@0/bl_power"
#define POWER_ON 0
#define POWER_OFF 1

#define BRIGHTNESS_FILE "/sys/class/backlight/backlight@0/brightness"
#define BRIGHTNESS_MAX 9
#define BRIGHTNESS_MIN 1
#define BRIGHTNESS_STEP 1

#define DEFAULT_CONTENT "3"
#define FILENAME "rpi-backlight"

int usage(char *argv[]) {
    printf("usage: %s up | down | max | min | on | off\n\n"
           "options:\n"
           "\tup:\t increases brightness by 10%%\n"
           "\tdown:\t decreases brightness by 10%%\n"
           "\tmax:\t sets brightness to 100%%\n"
           "\tmin:\t sets brightness to 0%%\n"
           "\ton:\t turns the screen on\n"
           "\toff:\t turns the screen off\n\n"
           "2015, Jakub Hladik, www.github.com/jakeh12\n\n"
           , argv[0]);
    
    return EXIT_FAILURE;
}

char *get_config_path() {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Could not find HOME environment variable.\n");
        exit(EXIT_FAILURE);
    }

    // ~/.config/bat
    char *path = malloc(strlen(home) + strlen("/.config/") + strlen(FILENAME) + 1);
    if (!path) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    sprintf(path, "%s/.config/%s", home, FILENAME);
    return path;
}

void ensure_file_exists(const char *filepath) {
    struct stat st;

    if (stat(filepath, &st) != 0) {
        // Create the file with default content
        FILE *f = fopen(filepath, "w");
        if (!f) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        fputs(DEFAULT_CONTENT, f);
        fclose(f);
        printf("Created file with default content: %s\n", filepath);
    }
}


void store_brightness_in_confg(int value){
	FILE *brightness_file;

	char *path = get_config_path();
	ensure_file_exists(path);

    brightness_file = fopen(path, "w");
    fprintf(brightness_file, "%d", value);
    fclose(brightness_file); 
	free(path);
}

int get_brightness_from_config(){
	FILE *brightness_file;
	int brightness_value;

    char *path = get_config_path();
    ensure_file_exists(path);
	
    brightness_file = fopen(path, "r");
    fscanf(brightness_file, "%d", &brightness_value);
    fclose(brightness_file);
	
	free(path);
    return brightness_value;

}
int get_brightness() {
    FILE *brightness_file;
    int brightness_value;
    
    brightness_file = fopen(BRIGHTNESS_FILE, "r");
    fscanf(brightness_file, "%d", &brightness_value);
    fclose(brightness_file);
    
    return brightness_value;
}

void set_brightness(int old_bright,int brightness_value) {

    if (brightness_value > BRIGHTNESS_MAX) brightness_value = BRIGHTNESS_MAX;
    if (brightness_value < BRIGHTNESS_MIN) brightness_value = BRIGHTNESS_MIN;
   

    if ( old_bright == brightness_value ) return;
	
	store_brightness_in_confg(brightness_value);

    FILE *brightness_file;
    
    brightness_file = fopen(BRIGHTNESS_FILE, "w");
    fprintf(brightness_file, "%d", brightness_value);
    fclose(brightness_file);
}

int get_power() {
    FILE *power_file;
    int power_value;
    
    power_file = fopen(POWER_FILE, "r");
    fscanf(power_file, "%d", &power_value);
    fclose(power_file);
    
    return power_value;
}

void set_power(int power_value) {
    if (power_value > POWER_OFF) power_value = POWER_ON;
    if (power_value < POWER_ON) power_value = POWER_OFF;
    
    FILE *power_file;
    
    power_file = fopen(POWER_FILE, "w");
    fprintf(power_file, "%d", power_value);
    fclose(power_file);
}

int main(int argc, char *argv[]) {
    
    // check if two arguments are passed in
    if (argc != 2) return usage(argv);
    
    // check if config files exist
    FILE *brightness_file;
    FILE *power_file;
    
    power_file = fopen(POWER_FILE, "r+");
    if (power_file == NULL) {
        printf("ERROR: '%s' does not exist/insufficient permissions.\n", POWER_FILE);
        return EXIT_FAILURE;
    }
    
    brightness_file = fopen(BRIGHTNESS_FILE, "r+");
    if (brightness_file == NULL) {
        printf("ERROR: '%s' does not exist/insufficient permissions.\n", BRIGHTNESS_FILE);
        return EXIT_FAILURE;
    }
    
    // everything ok, get current values
    int old_bright = get_brightness();
    int brightness_value = old_bright;
    int power_value = get_power();
    
    // process arguments
    if (strcmp(argv[1], "up") == 0) {
        brightness_value += BRIGHTNESS_STEP;
    }
    else if (strcmp(argv[1], "down") == 0) {
        brightness_value -= BRIGHTNESS_STEP;
    }
    else if (strcmp(argv[1], "max") == 0) {
        brightness_value = BRIGHTNESS_MAX;
    }
    else if (strcmp(argv[1], "min") == 0) {
        brightness_value = BRIGHTNESS_MIN;
    }
	else if (strcmp(argv[1],"sync") == 0) {
		brightness_value = get_brightness_from_config();
		
	}else if(strcmp(argv[1],"default") == 0){
		brightness_value = 3;
	}
    else if (strcmp(argv[1], "on") == 0) {
        power_value = POWER_ON;
    }
    else if (strcmp(argv[1], "off") == 0) {
        power_value = POWER_OFF;
    }
    else {
        return usage(argv);
    }
    
    // save new values
    set_brightness(old_bright,brightness_value);
    set_power(power_value);
    
    return EXIT_SUCCESS;
}
