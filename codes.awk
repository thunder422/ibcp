BEGIN { n = 0 }

$1 ~ /_Code/ {
	match($1,"[A-Za-z]*_Code")
	c[n] = substr($1,RSTART,RLENGTH)
	printf "%d: %s\n",i n, c[n]
	n++
}

END {
	print ""
	for (i = 0; i < n; i++)
		printf "%s = %d\n", c[i], i | "sort"
}
