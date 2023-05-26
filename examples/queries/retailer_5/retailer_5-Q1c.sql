IMPORT DTREE FROM FILE 'retailer_5-Q1c.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM q2_small (
    locn int,
    dateid int,
    ksn int
) FROM FILE './datasets/retailer/q2.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM Item(ksn int, subcategory int, category int, categoryCluster int, prize double)
FROM FILE './datasets/retailer/Item.tbl' LINE DELIMITED CSV(delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int]>](ksn) *
    [lift<1>: RingFactorizedRelation<[1,int, int]>](locn,dateid)
)
FROM q2_small NATURAL JOIN Item
