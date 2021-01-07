#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <errno.h>
#include<math.h>

/*
 * Explanation for code was from https://www.a1k0n.net/2011/07/20/donut-math.html
 * One thing of note is the fact that the matrices in the explanation are sideways
 * (presuming the first column normally represents x, second column y, and third column z).
 * In the explanation, the first row is x, second row is y, and third row is z.
 * This messed up my understanding for a second there.
 * */

#define screen_width 40
#define screen_height 40

const double theta_spacing = 0.07;
const double phi_spacing = 0.02;
const double alpha_spacing = 0.05;
const double beta_spacing = 0.05;

const double light_source[] = {0, -1, -1}; // Vector should have magnitude of sqrt(2).

/*
 * Question of the century:
 * What do you call the part of a donut excluding the hole?
 * 
 * This question has plagued philosophers for centuries - but today,
 * we answer that question here.
 *
 * We shall call it the dough.
 * */
const double r1 = 1; // Radius of the dough.
const double r2 = 3; // Distance from center of hole to center of dough.
const double scale = 3 / 7.0; // How much of the screen should be taken up.
const double k2 = 5; // Distance between donut and viewer.
const double k1 = screen_width * (r1 + r2) * scale / k2; // Fixed constant for scaling coordinates.

char* render_frame(double alpha, double beta) {
  // Precompute repeated values.
  double sin_a = sin(alpha), cos_a = cos(alpha);
  double sin_b = sin(beta), cos_b = cos(beta);

  // Create z_buffer for projecting from 3D to 2D.
  double z_buffer[screen_height][screen_width] = { 0 };
  // Create output and reset it.
  char *output = malloc(sizeof(char) * screen_height * screen_width);
  for (int i = 0; i < screen_height * screen_width; i++) {
    output[i] = ' ';
  }

  // First for loop creates the circle cross section of the dough.
  for (double theta = 0; theta < 2 * M_PI; theta += theta_spacing) {
    double sin_theta = sin(theta), cos_theta = cos(theta);
    double circle_x = r2 + r1 * cos_theta;
    double circle_y = r1 * sin_theta;

    // Second for loop creates the donut and then rotates it.
    for (double phi = 0; phi < 2 * M_PI; phi += phi_spacing) {
      // Do some more precalculations.
      double sin_phi = sin(phi), cos_phi = cos(phi);

      // These are the matrix values in the transformation matrix.
      double m00 = cos_b * cos_phi + sin_a * sin_b * sin_phi;
      double m01 = sin_b * cos_phi - sin_a * cos_b * sin_phi;
      double m10 = -1 * cos_a * sin_b;
      double m11 = cos_a * cos_b;
      double m02 = cos_a * sin_phi;
      double m12 = sin_a;
      // Do not need to calculate other matrix values as they are not used more than once.
      

      // Calculate the x and y coordinates of the transformed circle_x and circle_y
      double t_x = circle_x * m00 + circle_y * m10;
      double t_y = circle_x * m01 + circle_y * m11;
      double t_z = circle_x * m02 + circle_y * m12 + k2; // Have to include the distance between the donut and viewer.

      double z_i = 1 / t_z; // Invert t_z for efficiency and allows for "infinite z". 
  

      // Project 3D points to 2D plane and cast to integer.
      int xp = (int) (screen_width / 2.0 + k1 * z_i * t_x);
      int yp = (int) (screen_height / 2.0 - k1 * z_i * t_y); // Subtract here since y-axis is reversed on 2D planes when programming.


      // Check to see if a closer point at this x,y has already been plotted.
      if (z_i > z_buffer[yp][xp]) {
        // Save z to z_buffer.
        z_buffer[yp][xp] = z_i;

        // Calculate surface normal coords.
        double s_x = cos_theta * m00 + sin_theta * m10;
        double s_y = cos_theta * m01 + sin_theta * m11;
        double s_z = cos_theta * m02 + sin_theta * m12;

        // Do luminance calculations.
        double l_x = light_source[0];
        double l_y = light_source[1];
        double l_z = light_source[2];

        double luminance = l_x * s_x + l_y * s_y + l_z * s_z;
       
        // Since light vector has a magnitude of sqrt(2), then it ranges from [-sqrt(2), sqrt(2)]. We only care if the luminance is positive.
        if (luminance > 0) {
          int luminance_idx = luminance * 8; 
          // Luminance_idx ranges from 0 to 11. (8 * sqrt(2) = 11.3)
          output[yp * screen_width + xp] = ".,-~:;=!*#$@"[luminance_idx];
        }
      }
    }
  }

  return output;
}

int msleep(long msec) {
  struct timespec ts;
  int res;
  if (msec < 0) {
    errno = EINVAL;
    return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);

  return res;
}

int main() {
  double alpha = 0, beta = 0;
  while (1) {
    alpha += alpha_spacing;
    beta += beta_spacing;
    char *frame = render_frame(alpha, beta);

    printf("\x1b[H");
    for (int j = 0; j < screen_height; j++) {
      for (int i = 0; i < screen_width; i++) {
        putchar(frame[j * screen_width + i]);
      }
      putchar('\n');
    }

    msleep(100);
    free(frame);
  }
}
