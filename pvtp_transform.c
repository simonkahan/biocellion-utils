// Usage: a.out <infile.pvtp> 
//
// Utility to transform pvtp & vtp files produced by Biocellion simulator into the format Biovision reads.
// Biovision format requires radius, color, scale, orient, and points data.
// This utility assumes radius data - and only radius data - is absent and prepends data with radius = 1.0
//  for all points.
// Produces bv_<infile.pvtp> and for each <file.vtp> mentioned in <infile.pvtp> a bv_<file.vtp>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define OPEN_FILE(file,filename,mode)  \
FILE * file = fopen(filename,#mode); \
 if (file) {fprintf(stderr, "opening %s\n", filename);}	\
 else {fprintf(stderr, "couldn't open %s\n", filename), exit(0);}

void transform_vtp(char *filename);
int main(int argc, char** argv) {
  char * filename = argv[1];
  OPEN_FILE(file,filename,r);
  char * bv_filename = alloca(strlen(filename)+3);
  strcpy(bv_filename,"bv_");
  strcat(bv_filename,filename);
  OPEN_FILE(bv_file_pvtp,bv_filename,w);
  char * line = NULL;
  int numpoints = 0;
  int radius_header_line_added = 0;
  int data_offset = 0;
  size_t read;
  while (getline(&line,&read,file)>=0) {
    /* write out header line */
    if (strstr(line, "<PDataArray>")) {
      if (!radius_header_line_added) {
	fprintf(bv_file_pvtp,"<PDataArray type=\"Float64\" Name=\"radius\" format=\"appended\" />\n");
	radius_header_line_added = 1;
      }
    }
    if (strstr(line, "Source")) {
      const char delimiters[] = "\"";
      char *token, *cp;
      cp = strdup (line);
      token = strtok(cp,delimiters);
      char * input_vtp = strtok(NULL,delimiters);
      transform_vtp(input_vtp);
      free(cp);
      fprintf(bv_file_pvtp,"<Piece Source=\"bv_%s\"/>\n",input_vtp);
      continue;
    }
    fprintf(bv_file_pvtp,"%s", line);
  }
}

void transform_vtp(char *filename) {
  OPEN_FILE(file,filename,r);
  char * bv_filename = alloca(strlen(filename)+3);
  strcpy(bv_filename,"bv_");
  strcat(bv_filename,filename);
  OPEN_FILE(bv_file_vtp,bv_filename,w);

  char * line = NULL;
  int numpoints = 0;
  int radius_header_line_added = 0;
  int data_offset = 0;
  while (1) {
    size_t read;
    getline(&line, &read, file);
    if (strstr(line, "NumberOfPoints") != NULL) {
      char * c; for (c = line ; *c != '"' ; c++);
      numpoints = atoi(c+1);
      fprintf(stderr, "reading %d points.\n", numpoints);
    }
    /* write out header line */
    if (strstr(line, "offset")) {
      if (!radius_header_line_added) {
	fprintf(bv_file_vtp,"<DataArray type=\"Float64\" Name=\"radius\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
	data_offset += 8*numpoints+4;
	radius_header_line_added = 1;
      }
      if (strstr(line, "color")) {
	fprintf(bv_file_vtp,"<DataArray type=\"Float64\" Name=\"color\" format=\"appended\" offset=\"%d\"/>\n",data_offset);
	data_offset += 8*numpoints+4;
      } else if (strstr(line, "scale")) {
	fprintf(bv_file_vtp,"<DataArray type=\"Float64\" Name=\"scale\" NumberOfComponents=\"3\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
	data_offset += 24*numpoints+4;
      } else if (strstr(line, "orient")) {
	fprintf(bv_file_vtp,"<DataArray type=\"Float64\" Name=\"orient\" NumberOfComponents=\"3\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
	data_offset += 24*numpoints+4;
      } else {
	fprintf(bv_file_vtp,"<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"appended\" offset=\"%d\"/>\n", data_offset);
      }
    } else fprintf(bv_file_vtp,"%s", line);
    if (strcmp(line, "<AppendedData encoding=\"raw\">\n")==0) break;
  }
  int c,char_count=0;
  while ((c = getc (file))!=EOF) {
    if (c == '_' && char_count == 0) {
      putc(c,bv_file_vtp);
      fwrite((uint32_t*)&numpoints,4,1,bv_file_vtp);
      for (int i = 0; i < numpoints; i++) {
	double one = 1.0;
	fwrite(&one,8,1,bv_file_vtp);
      }
    }
    else putc(c,bv_file_vtp);
    if (!(++char_count&0xffff))fprintf(stderr, "%d chars read\n",char_count);
  }
}
