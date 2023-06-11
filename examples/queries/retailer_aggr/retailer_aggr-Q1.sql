IMPORT DTREE FROM FILE 'retailer_aggr-Q1.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM Inventory(locn int, dateid int, ksn int, inventoryunits int)
FROM FILE './datasets/retailer_unordered/Inventory.tbl' LINE DELIMITED CSV(delimiter := '|');


SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int,int]>](dateid,locn) *
    [lift<2>: RingFactorizedRelation<[2,int]>](ksn)
)
FROM Inventory;