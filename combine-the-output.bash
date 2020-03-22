{
eselect python list | head -n2 | tail -n1 | awk '{print $2}';
eselect gcc list | awk '{print $2}';
eselect kernel list | tail -n1 | awk '{print $2}';
} | paste -s -d "\t" -
