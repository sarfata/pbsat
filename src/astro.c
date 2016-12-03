#include "astro.h"

/* Gets the azimuth and elevation of an object in the sky
 * given its right ascension and declination
 * for a given observer at lat/lon and at utc time t.
 *
 * All degrees are expressed in Pebble Degrees (TRIG_MAX_ANGLE).
 * Built from the JS at: http://www.convertalot.com/celestial_horizon_co-ordinates_calculator.html
 */
/*
void coord_to_horizon(int32_t ra, int32_t dec, int32_t lat, int32_t lon, time_t utc, int32_t *az, int32_t *el) {
  // compute hour angle in degrees
  int32_t ha = mean_sidereal_time(utc, lon) - (ra * 360) / TRIG_MAX_ANGLE;
  if (ha < 0) {
    ha = ha + TRIG_MAX_ANGLE;
  }
  int32_t mean_sidereal_time = f(t);
  int32_t local_mean_sidereal_time = 
}
*/


// WHAT I AM DOING???
// -> Calculate the elevation of the sun for any lat/lon
// so that I can color the world map


double compute_sun(time_t utc, double lon) {
  // E = M + e*(180/pi) * sin(M) * ( 1.0 + e * cos(M) )
  double d = epoch_delta(utc);
  M = 356.0470 + 0.9856002585 * d;

  double E = 
}

double mean_sidereal_time(time_t utc, double lon) {
  double jd = epoch_delta(utc);
  double jt = jd / 36525.0;

  double mst = 280.46061837 + 360.98564736629*jd + 0.000387933*jt*jt - jt*jt*jt/38710000.0 + lon;

  if (mst > 0.0) {
    while (mst > 360.0) {
      mst -= 360.0;
    }
  }
  else {
    while (mst < 0.0) {
      mst += 360.0;
    }
  }
  return mst;
}

double epoch_delta(time_t utc) {
  struct tm *date = gmtime(&utc);

  int year = date->tm_year + 1900;
  int month = date->tm_mon + 1;
  int day = date->tm_mday;

  // the last term of the equation is moved to the return line because it uses float
  int d = 367 * year - 7 * (year + (month + 9) / 12) / 4 + 275 * month /9 + day;
  int seconds = date->tm_hour * 3600 + date->tm_min * 60 + date->tm_sec;

  return (double)d - 730531.5 + ((double)seconds) / (24 * 3600);
}

bool compare_doubles(char *label, double result, double expected) {
  double error = expected - result;

  if (error > 0.1 || error < -0.1) {
    int result_frac = (result - (int)result) * 1000;
    int expected_frac = (expected - (int)expected) * 1000;
    printf("FAIL: %s - Got %d.%03d, expected %d.%03d error*1000=%d", 
        label, (int)result, result_frac, (int)expected, expected_frac, (int)(error*1000));
    //printf("FAIL: %s - Got %f, expected %f", label, result, expected);
    //printf("FAIL: %s - Got %s, expected %s", label, s1, s2);
    return false;
  }
  else {
    printf("PASS: %s", label);
    return true;
  }
}

void test_epoch_delta() {
  const time_t test_date = 1444527480;
  const double expected = 5761.5680555555555;

  double result = epoch_delta(test_date);

  compare_doubles("epoch_delta", result, expected);
}

void test_mean_sidereal_time() {
  const time_t test_date = 1444527480;
  const double test_longitude = -122;
  double expected = 281.8350076817442;

  double result = mean_sidereal_time(test_date, test_longitude);

  compare_doubles("mean_sidereal_time", result, expected);
}

void test_astro() {
  test_epoch_delta();
  test_mean_sidereal_time();
}

