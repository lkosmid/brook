#!/usr/bin/perl

for($i=0;$i<1024;$i=$i+1){
  print ($i);
  print (" ");
  print `../../../bin/tessellate.exe .00000000000001 $i`;
  print ("\n");
}