IMPORT DTREE FROM FILE 'rst-Q1.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM R(A int, B float) FROM FILE './datasets/simple/R.dat' LINE DELIMITED CSV;
CREATE TABLE S(A int, C int, E float, extra int) FROM FILE './datasets/simple/S.dat' LINE DELIMITED CSV;
CREATE STREAM T(A int, C int) FROM FILE './datasets/simple/T.dat' LINE DELIMITED CSV;

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int]>](A)*
    [lift<1>: RingFactorizedRelation<[1,float]>](B)*
    [lift<2>: RingFactorizedRelation<[2,int]>](C))
    FROM R NATURAL JOIN S NATURAL JOIN T;