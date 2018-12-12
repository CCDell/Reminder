#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <stdio.h>
#include <syslog.h>
#include <gtk/gtk.h>

#define SECS_P_HR 5
#define WIND_HEIGHT 200
#define WIND_LENGTH 100
#define GRID_LENGTH 8
#define GRID_POSX 0
#define GRID_POSY 0
#define TEXT_BOX_POSX 0
#define TEXT_BOX_POSY 8  
#define TEXT_BOX_LENGTH 1
#define BUTTON_POSX 1
#define BUTTON_POSY 8
#define BUTTON_LEN 1
#define MAXSTR 1000
#define MAXDATA 5
#define MAXTODO 100

typedef struct {
  
  GtkWidget* widgets[MAXDATA];
  int widgetCount;
  char * strings[MAXTODO];
  int stringCount;
  
} userInfo;


void activate(GtkApplication *, gpointer);
void getPNGRatio(char * filename, int *height , int * width);
void on_button_clicked(GtkButton * button, gpointer user_data);
int getNextHour(time_t * timeStart);
int getDelimitedStrings(char * strlist[], char * tempStr, const char * delim,
			const int strlist_len);
int readFileStrings(const char * filename, char *  todo[], const int todoLen,
	     const int strlen, const char * delim);

int main(int argc, char * argv[]){
  time_t timeStart;
  int nxtHour;
  // if(daemon(1, 0) == -1) error(1, 1, "main.c daemon error");
  
  time(&timeStart); // Get starting time of daemon
  
  // Get next closest hour
  nxtHour = getNextHour(&timeStart);
  
  short tempSig = -1;
  
  while(1){
    time_t t; // Get current time, or end time for calculations to see if 1 hr has passed
    time(&t);
    
    if(tempSig == -1){
      tempSig = 1;
    }
    
    if(t % SECS_P_HR == 0){
      tempSig = -1;
      GtkApplication * app; // Main application to be run
      GtkWidget* window = NULL;  // Window is a portion of the application, application
      // can have many windows
      GtkWidget* image = NULL; // Image in window
      userInfo * data = malloc(sizeof(userInfo));
      bzero(data, sizeof(userInfo));

      int status = 1; // To see if thin has failed
      
      app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE); // makes a new app with ame HR
      
      g_signal_connect (app, "activate", G_CALLBACK (activate), data); // Connects the app
      //to the callback activate.

      status = g_application_run(G_APPLICATION (app), argc, argv); // Runs the application.
      g_object_unref(app); // deletes app when done
      
      for(int i = 0; i < data->stringCount; ++i)
	free(data->strings[i]);
      free(data);
      exit(0);
    }
  }

  return 0;
}

// When app is made, this is called to fill the window
void activate(GtkApplication *app, gpointer user_data){
  
  // Window is apart of application, image is a part of window
  userInfo * data =  user_data;
  GtkWidget * window = NULL; 
  GtkWidget * image = NULL;
  GtkWidget * grid = NULL;
  GtkWidget * entry = NULL;
  GtkWidget * button = NULL;
  GtkWidget * label = NULL;
  GtkEntryBuffer * buff = NULL;

  // Get image's dimensions
  int image_height = 0;
  int image_width = 0;
  getPNGRatio("quoteCrop.png", &image_height, &image_width);

  // Get the information stored in the todo file

  data->stringCount = readFileStrings("todo.txt", data->strings, MAXTODO, MAXSTR, "=*=");
  
  window = gtk_application_window_new(app); // Make a new window under the app
  gtk_window_set_title(GTK_WINDOW(window), "HR"); // Title the window
  gtk_window_set_resizable(GTK_WINDOW(window), 0);
  gtk_window_set_default_size(GTK_WINDOW(window), WIND_HEIGHT, WIND_LENGTH);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  
  image = gtk_image_new_from_file("quoteCrop.png");
  grid = gtk_grid_new(); // Make grid
  entry = gtk_entry_new();// For getting entered text
  button = gtk_button_new_with_label("Okay");
  buff = gtk_entry_buffer_new("", 0);
  label = gtk_label_new("");
  
  gtk_entry_buffer_set_max_length(buff, 100);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter something...");

  
  data->widgets[0] = label;
  data->widgets[1] = entry;
  data->widgetCount +=2;
  
  g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), data);
  
  /// The last 2 arguments are the grid
  // spaces it possesses, because it does not change the window's height, the grid's rows
  // and height divide the length and width of the window by the last parameters  
  gtk_grid_attach(GTK_GRID(grid), image, GRID_POSX, GRID_POSY, GRID_LENGTH, GRID_LENGTH);
  gtk_grid_attach(GTK_GRID(grid), entry,
    TEXT_BOX_POSX,TEXT_BOX_POSY, TEXT_BOX_LENGTH,TEXT_BOX_LENGTH);
  gtk_grid_attach(GTK_GRID(grid), button,
    BUTTON_POSX, BUTTON_POSY, BUTTON_LEN, BUTTON_LEN);
  gtk_grid_attach(GTK_GRID(grid), label, 10,10,10,10);
  
  gtk_container_add(GTK_CONTAINER (window), grid);
  gtk_widget_show_all(window);
  


}

void getPNGRatio(char * fileName, int * height, int * width){
 
  const char * readType = "rb"; // set read type to read byte
  FILE * file = fopen(fileName, readType); // Opens file for reading
  const int timesRead = 2; // Times to read 4 bytes
  const int startLocationInFile = 16; // The resolution starts 16 bytes into file, that's all we want

  int dimensions[2] = {};
  fseek(file, startLocationInFile, SEEK_SET); // seeks startLocationInFile bytes after the start (SEEK_SET) which is where the resolution is
  
  for(int i = 0; i < timesRead; i++){ // Iterates over amount of times 4 bytes are read
    // (When timesRead = 2, 8 bytes are read as 4 * 2 = 8)
    char byteList[sizeof(int)] ={}; // List of bytes, which is four bytes long
    unsigned int total = 0; // Will be the integer represented by byteList

    fread(byteList, sizeof(char), sizeof(int), file); // Reads sizeof(int) (4) chunks of
    // size sizeof(char) (1) into byteList from file
    
    // Combine single bytes into 4 byte integer
    total += (byteList[0] << 24) + (byteList[1] << 16) +
      (byteList[2] << 8) + (byteList[3]);
    
    dimensions [i] = total; //First spot is width, second is height based on PNG file header standards
  }

  *width = dimensions[0];
  *height = dimensions[1];
  fclose(file);
}

void on_button_clicked(GtkButton * button, gpointer user_data){
  
  /// for either of these initializations, treat the user_data as an array filled with only
  // the type desired. to get a label, say the entire user_data array is of GtkLabels.
  // this is because the gpointer is a void * and does not like to be dereferenced any
  // other way. gtk_entry_get_text() for getting data from entry
  
  userInfo * data = user_data;
  
  GtkLabel * label = (GtkLabel*) data->widgets[0]; 
  GtkEntry * entry = (GtkEntry*) data->widgets[1];
  
  for(int i = 0 ; i < data->stringCount; ++i){
    gtk_label_set_text(label,data->strings[i]);
  }
  
  gtk_entry_set_text(entry, "");
  
}

int readFileStrings(const char * filename, char * strlist[],
		    const int strlist_len, const int strlen, const char * delim){
  
  FILE * file = NULL;
  int num_read = 0;
  const char * readType = "r";
  char tempChar = 0;
  char tempStr[MAXSTR] = {};
  int charRead = 0;
  
  file = fopen(filename, readType);
  
  // Read entire file
  
  while((tempChar = fgetc(file)) != EOF)
    tempStr[charRead++] = tempChar;
  tempStr[charRead-1] = '\0';
  
  // Copy delimited areas into strlist
  num_read = getDelimitedStrings(strlist, tempStr, delim, strlen);

  fclose (file);
  return num_read;
}

int getDelimitedStrings(char * strlist[], char * tempStr, const char * delim,
			const int strlist_len){

  int num_read = 0;
  char * temp = NULL;
  if((temp = strtok(tempStr, delim)) != NULL){

    strlist[num_read] = malloc(strlen(temp)+1);
    strcpy(strlist[num_read], temp);
    
    for(num_read = 1; num_read < strlist_len &&
	  (temp = strtok(NULL, delim)) != NULL; ++num_read){
      strlist[num_read] = malloc(strlen(temp)+1);
      strcpy(strlist[num_read], temp);
    }
  }

  return num_read;

}


int getNextHour(time_t * timeStart){
  
  for(int i = 0; i < SECS_P_HR+1; ++i){
    if((*timeStart + i) % SECS_P_HR == 0){
      return i;
    }
  }
    
}
