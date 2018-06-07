// Usage: a.out <infile.vtp> > <outfile.vtp>
//
// Utility to transform vtp files produced by Biocellion simulator into the format Biovision reads.
// Biovision format requires radius, color, scale, orient, and points data.
// This utility assumes radius data - and only radius data - is absent and prepends data with radius = 1.0
//  for all points.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
int main(int argc, char** argv) {
  FILE * file = fopen(argv[1],"r");
  fprintf(stderr, "opening %s\n", argv[1]);
  char * line = NULL;
  int numpoints = 0;
  int radius_header_line_added = 0;
  int data_offset = 0;
  while (1) {
    ssize_t read;
    getline(&line, &read, file);
    if (strstr(line, "NumberOfPoints") != NULL) {
      char * c; for (c = line ; *c != '"' ; c++);
      numpoints = atoi(c+1);
      fprintf(stderr, "reading %d points.\n", numpoints);
    }
    /* write out header line */
    if (strstr(line, "offset")) {
      if (!radius_header_line_added) {
	printf("<DataArray type=\"Float64\" Name=\"radius\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
	data_offset += 8*numpoints+4;
	radius_header_line_added = 1;
      }
      if (strstr(line, "color")) {
	printf("<DataArray type=\"Float64\" Name=\"color\" format=\"appended\" offset=\"%d\"/>\n",data_offset);
	data_offset += 8*numpoints+4;
      } else if (strstr(line, "scale")) {
	printf("<DataArray type=\"Float64\" Name=\"scale\" NumberOfComponents=\"3\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
	data_offset += 24*numpoints+4;
      } else if (strstr(line, "orient")) {
	printf("<DataArray type=\"Float64\" Name=\"orient\" NumberOfComponents=\"3\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
	data_offset += 24*numpoints+4;
      } else {
	printf("<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
      }
    } else printf("%s", line);
    if (strcmp(line, "<AppendedData encoding=\"raw\">\n")==0) break;
  }
  int c,char_count=0;
  while ((c = getc (file))!=EOF) {
    if (c == '_' && char_count == 0) {
      putc(c,stdout);
      fwrite((uint32_t*)&numpoints,4,1,stdout);
      for (int i = 0; i < numpoints; i++) {
	double one = 1.0;
	fwrite(&one,8,1,stdout);
      }
    }
    else putc(c,stdout);
    if (!(++char_count&0xffff))fprintf(stderr, "%d chars read\n",char_count);
  }
}
