IMPORT DTREE FROM FILE 'retailer_3-Q2.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM Inventory(locn int, dateid int, ksn int, inventoryunits int)
FROM FILE './datasets/retailer_unordered/Inventory.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM Item(ksn int, subcategory int, category int, categoryCluster int, prize double)
FROM FILE './datasets/retailer_unordered/Item.tbl' LINE DELIMITED CSV(delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int]>](ksn) *
    [lift<1>: RingFactorizedRelation<[1,int,int]>](locn, dateid) *
    [lift<5>: RingFactorizedRelation<[5,double,int]>](prize,category)
)
FROM Inventory NATURAL JOIN Item;
