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
#define MAXSTR 100
#define MAXDATA 5

void getPNGRatio(FILE* filename, int *height , int * width);
void activate(GtkApplication *, gpointer);
void on_button_clicked(GtkButton * button, gpointer user_data);

int main(int argc, char * argv[]){
  time_t timeStart;
  int nxtHour;
  // if(daemon(1, 0) == -1) error(1, 1, "main.c daemon error");
  
  time(&timeStart); // Get starting time of daemon
  
  // Get next closest hour
  for(int i = 0; i < SECS_P_HR+1; ++i){
    if((timeStart + i) % SECS_P_HR == 0){
      nxtHour = i;
      break;
    }
  }
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
      GtkWidget * data[MAXDATA] = {};

      int status = 1; // To see if thin has failed
      
      app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE); // makes a new app with ame HR
      
      g_signal_connect (app, "activate", G_CALLBACK (activate), data); // Connects the app
      //to the callback activate.

      status = g_application_run(G_APPLICATION (app), argc, argv); // Runs the application.
      g_object_unref(app); // deletes app when done
      
    }
  }
    
  return 0;
}

// When app is made, this is called to fill the window
void activate(GtkApplication *app, gpointer user_data){
  
  // Window is apart of application, image is a part of window
  GtkWidget ** data = user_data;
  GtkWidget * window = NULL; 
  GtkWidget * image = NULL;
  GtkWidget * grid = NULL;
  GtkWidget * entry = NULL;
  GtkWidget * button = NULL;
  GtkWidget * label = NULL;
  GtkEntryBuffer * buff = NULL;

  FILE * file = fopen("quoteCrop.png", "rb"); // Opens file for reading
  int image_height = 0;
  int image_width = 0;
  getPNGRatio(file, &image_height, &image_width);
  fclose(file);
  
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

  data[0] = label;
  data[1] = entry;
  
  g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), (void *) data);
  
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

void getPNGRatio(FILE * file, int * height, int * width){
  

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
}

void on_button_clicked(GtkButton * button, gpointer user_data){

  /// for either of these initializations, treat the user_data as an array filled with only
  // the type desired. to get a label, say the entire user_data array is of GtkLabels.
  // this is because the gpointer is a void * and does not like to be dereferenced any
  // other way
  GtkLabel * label = ((GtkLabel **)user_data)[0]; 
  GtkEntry * entry = ((GtkEntry **)user_data)[1];
   
  gtk_label_set_text(label, gtk_entry_get_text(entry));
  
  gtk_entry_set_text(entry, "");
  
}
