

my $level=0;
my $targetlvl=0;
my $transistionRate=0;




 transistion( $level, $targetlvl);

 $targetlvl = 159;
 transistion( $level, $targetlvl);


 $targetlvl = 0;
 transistion( $level, $targetlvl);


 print("transistionRate = 2\n");
 $targetlvl = 159;
 $transistionRate = 2;
 transistion( $level, $targetlvl);

 print("transistionRate = 2\n");
 $targetlvl = 0;
 $transistionRate = 2;
 transistion( $level, $targetlvl);

 print("transistionRate = 3\n");
 $targetlvl = 159;
 $transistionRate = 3;
 transistion( $level, $targetlvl);

 print("transistionRate = 3\n");
 $targetlvl = 0;
 $transistionRate = 3;
 transistion( $level, $targetlvl);


 print("transistionRate = 4\n");
 $targetlvl = 159;
 $transistionRate = 4;
 transistion( $level, $targetlvl);

 print("transistionRate = 4\n");
 $targetlvl = 0;
 $transistionRate = 4;
 transistion( $level, $targetlvl);



 print("transistionRate = 4\n");
 $level   = 79;
 $targetlvl = 80;
 $transistionRate = 4;
 transistion( $level, $targetlvl);

 print("transistionRate = 4\n");
 $level   = 81;
 $targetlvl = 80;
 $transistionRate = 4;
 transistion( $level, $targetlvl);


 print("transistionRate = 4\n");
 $level   = 80;
 $targetlvl = 80;
 $transistionRate = 4;
 transistion( $level, $targetlvl);




sub transistion( ) {

  do {
    my $newlvl = trans( $level, $targetlvl);
    print( "T:$transistionRate Level: $level \t Target: $targetlvl \t New Level: $newlvl  \n"); 
    $level = $newlvl;
    print("" );

  } until( $transistionRate == 0);

}




sub trans( ) {
  my $returnlvl;

  if ($transistionRate <2) {
    $transistionRate=0;
    return ( $targetlvl) 
  };

  #  $returnlvl = int(($targetlvl - $level) / $transistionRate)+$level;

  if ( $level > $targetlvl) {
    $returnlvl = $level - int ( ($level - $targetlvl) / $transistionRate+1);


  }
  elsif ( $level < $targetlvl) {
    $returnlvl = int(($targetlvl - $level) / $transistionRate+1)+$level;

  } else {
    $returnlvl = $targetlvl;
  }

  $transistionRate--;

  return $returnlvl;

  
}
