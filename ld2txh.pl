#Make an associative array for parts of the procedure to its category
%Sect=('lfn',  "long file name",
       'long', "long file name",  
       'sfn',  "short file name",
       'short', "short file name",
       'search',"handle",
       'handle',"handle",
       'str',    'strings',
       'toupper','strings',
       'xms',    'extended memory',
       'sector', 'sector',
       'dir',    'directory',
       'path',   'path',
       'cluster',"cluster",
       'disk',   'disk',
       'fat',    'disk',
       'drive',  'disk',
       'partiti','disk',
       'dpb',    'disk'
      );

open(F, "<lfn.doc");
foreach (<F>)
{
   if (/\s*;\s*([^\n]+)/)
     {
        # Print out this bit of the docs
        if ($1 eq "in" || $1 eq "out")
          {
            if ($InTable) {print "\@end table\n";}
            print "\@subheading $1\n";
            print "\@table \@code\n";
            $InTable=1;
          }
         elsif ($InTable && (/([\w:]+)=([^=\n]+)\n/ || /([\w:]+)\s*->([^\n]+)\n/))
          {print "\@item $1\n$2\n";}
         elsif ($InTable && /([\w:]+)!=([^=\n]+)\n/)
          {print "\@item $1\n != $2\n";}
         elsif ($InTable && /([\w:]+)==([^=\n]+)\n/)
          {print "\@item $1\n == $2\n";}
         elsif (/([^\n]*)\(([a-z]{2,}_[a-z_]*)\)([^\n]*)/ ||
               /([^\n]*)`([a-z]{2,}_[a-z_]*)'([^\n]*)/)
          {print "$1 \@code{$2} (\@pxref{$2}) $3\n";}
         else
          {print "$1\n";}
     }
   elsif (/([^:\n\s]+):.*/)
     {
         if ($InTable) {print "\@end table\n"; $InTable = 0;}

         # Next try to figure out the section
         $Minor=$1;
         $_=$1;
         $Maj="misc"; # Default to misc category
         foreach $K (keys %Sect)
          {if (/$K/) {$Maj = $Sect{$K};}}

          #Print out the txh header
          print "\@node $Minor, $Maj\n";
     }
}
if ($InTable) {print "\@end table\n"; $InTable = 0;}
close F;
