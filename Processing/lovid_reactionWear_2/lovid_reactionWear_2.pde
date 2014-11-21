import processing.video.*;
/*************************************************
 *LoVid - ReactionWear                            *
 *Processing code by Bryan Ma - bryanhma@gmail.com*
 *************************************************/

Movie[] part = new Movie[7]; 
public int playing = 0; 
public boolean contact = false; 
public boolean nextMovie = false;
public int state = 0; 
public int bufferCount = 0; 
public int flashCounter = 0; 
public int rgbTrack = 0; 
public int stateCounter = 0; 
public int shrink = 0; 
public boolean rgbDisplay = false; 
//state 0 - video 1, searching for contact (no contact)
//state 1 - bubbles, RGB flashes if lose contact (both)
//state 2 - inside/outside bubbles, switch if lose contact (both)
//state 3 - outer bubble turns to line (no contact) 
//state 4 - tentacles form with new contact (contact) and glitch??

public static int BUFFER_LIMIT = 100; //100 frames for waiting for RGB trigger
public static int FLASH_COUNT = 30; //30 frames per RGB flash
public static int NO_COMM_LIMIT = 300; //10 seconds

public void setup() {
  size(displayWidth, displayHeight, P2D); 

  part[0] = new Movie(this, "part1_reactionwear_EDIT.mov");  
  part[1] = new Movie(this, "movingcircles.mp4"); 
  part[2] = new Movie(this, "pictureinpicture2.mov"); 
  part[3] = new Movie(this, "pictureinpicture2_flip.mov"); 
  part[4] = new Movie(this, "part3____5_EDIT.mov"); 
  part[5] = new Movie(this, "part3___5_EDIT.mov"); 
  part[6] = new Movie(this, "tenticles.mov"); 

  part[playing].loop(); 
  frameRate(30);
  noStroke();
}

public void draw() { 
  background(0);
  println("part 1 avail?: " + part[0].available() + 
    "\npart 2 avail?: " + part[1].available() + 
    "\npart 3 avail?: " + part[2].available() + 
    "\npart 4 avail?: " + part[3].available() + 
    "\npart 5 avail?: " + part[4].available() + 
    "\npart 6 avail?: " + part[5].available() + 
    "\npart 7 avail?: " + part[6].available() + "\n" +
    "current state: " + state + "\n" + 
    "has contact?: " + contact + "\n"); 

  switch(state) {
  case 0 : 
    if ( part[playing].available() ) {
      part[playing].read();
    }
    image(part[playing], 0, 0, width, height);
    if (contact) { 
      changeMovie(playing, playing+1, 1);
      state = 1;
    } 
    break;
  case 1 :
    background(255); 
    if ( part[playing].available() ) {
      part[playing].read();
    }
    image(part[playing],0,0,width,height); 
    fill(255, 255, 255, 100); 
    if (!contact) { 
      bufferCount ++; 
      println("buffer is at " + bufferCount);
    } else {
      bufferCount = 0; 
      rgbTrack = 0; 
      flashCounter = 0;
      println("buffer is at " + bufferCount); 
      rgbDisplay = false;
    } 
    if (bufferCount > BUFFER_LIMIT) {
      if (!rgbDisplay) {
        rgbDisplay = true;
        if (rgbTrack == 0) {
          rgbTrack++;
        }
      } else {
        switch (rgbTrack) {
        case 1: 
          if (flashCounter % 6 == 0) {
            fill(255, 0, 0, 255);
          } else if ((flashCounter+1) % 6 == 0) {
            fill(0, 255, 0, 255);
          } else if ((flashCounter+2) % 6 == 0) {
            fill(0, 0, 255, 255);
          }
          flashCounter++;
          if (flashCounter > FLASH_COUNT) {
            rgbDisplay = false;
            rgbTrack++; 
            bufferCount = 0;
            flashCounter = 0;
          } 
          break;
        case 2: 
          if (flashCounter % 5 == 0) {
            fill(255, 0, 0, 255);
          } else if ((flashCounter+1) % 5 == 0) {
            fill(0, 255, 0, 255);
          } else if ((flashCounter+2) % 5 == 0) {
            fill(0, 0, 255, 255);
          }
          flashCounter++;
          if (flashCounter > FLASH_COUNT) {
            rgbDisplay = false;
            rgbTrack++; 
            bufferCount = 0;
            flashCounter = 0;
          } 
          break;
        case 3: 
          if (flashCounter % 4 == 0) {
            fill(255, 0, 0, 255);
          } else if ((flashCounter+1) % 4 == 0) {
            fill(0, 255, 0, 255);
          } else if ((flashCounter+2) % 4 == 0) {
            fill(0, 0, 255, 255);
          }
          flashCounter++;
          if (flashCounter > FLASH_COUNT) {
            rgbDisplay = false;
            rgbTrack++; 
            bufferCount = 0;
            flashCounter = 0;
          } 
          break;
        case 4: 
          changeMovie(playing, playing+1, 2);
          state = 2;
          break;
        }
        rect(0, 0, width, height); 
        println("rgbTrack is " + rgbTrack);
      }
    }
    break;
  case 2 :
    if ( part[playing].available() ) {
      part[playing].read();
    } 
    if (part[playing+1].available()) {
      part[playing+1].read();
    }
    if (contact) {
      stateCounter = 0;
      image(part[playing],0,0,width,height);      
    } else {
      stateCounter++;
      image(part[playing+1],0,0,width,height);
    } 
    if (stateCounter > NO_COMM_LIMIT) { 
      state = 3;
      changeMovie(playing, playing+2, 2);
    } 
    break;
  case 3 :
    if ( part[playing].available() ) {
      part[playing].read();
    } 
    if (part[playing+1].available()) {
      part[playing+1].read();
    }
    image(part[playing+1], width/5, height/5+shrink/2, width-(width/5)*2, (height-(height/5)*2)-shrink);
    image(part[playing], width/3, height/3, width-(width/3)*2, (height-(height/3)*2));
    
    if (((height-(height/5)*2)-shrink) > 1) {
      shrink++;
    } else {
      println("playing " + playing);
      state = 4;   //<>//
      //changeMovie(playing,playing+2,1);
      part[playing+2].loop();
    } 
    println(((height-(height/5)*2)-shrink));

    
    break;
  case 4 :
  if ( part[playing].available() ) {
      part[playing].read();
    } 
    if (part[playing+1].available()) {
      part[playing+1].read();
    }
    if ( part[playing+2].available() ) {
      part[playing+2].read();
    }
    image(part[playing+1], width/5, height/5+shrink/2, width-(width/5)*2, (height-(height/5)*2)-shrink);
    image(part[playing], width/3, height/3, width-(width/3)*2, (height-(height/3)*2));
    if (contact) {
      fill(255);
      textSize(40);
      text("TENTACLES!", 250, height/2);
      image(part[playing+2], 0, 0, width, height);
    }
    break;
  }

  if (nextMovie) {
    changeMovie(playing, playing+1, 1);
  } 

  nextMovie = false;
}

public void changeMovie(int oldMovieNum, int newMovieNum, int numMovies) {
  if (newMovieNum == 7) {
    newMovieNum = 0;
  }
  part[oldMovieNum].stop(); 
  part[newMovieNum].loop();
  playing = newMovieNum;
  if (numMovies == 2 && newMovieNum < 7) { 
    part[newMovieNum+1].loop();
  }
  if (newMovieNum - oldMovieNum == 2) {
    part[oldMovieNum+1].stop(); 
  } 
} 

public void keyPressed() {
  if (key == ' ') {
    if (!contact) {
      contact = true;
    }
  } else if (key == 'n') {
    if (!nextMovie) {
      nextMovie = true;
    }
  }
} 

public void keyReleased() {
  if (key == ' ') { 
    if (contact) {
      contact = false;
    }
  } else if (key == 'n') {
    if (nextMovie) {
      nextMovie = false;
    }
  }
} 

//
//void movieEvent(Movie m) {
//  m.read();
//}
