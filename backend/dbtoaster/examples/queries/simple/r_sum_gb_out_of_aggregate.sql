CREATE STREAM R(A int, B int)
FROM FILE 'examples/data/simple/r.dat' LINE DELIMITED
CSV ();

SELECT A+SUM(B) FROM R GROUP BY A