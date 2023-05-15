IMPORT DTREE FROM FILE 'retailer_4Q1b.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM INVENTORY(locn int, dateid int, ksn int, inventoryunits int)
FROM FILE './datasets/retailer/Inventory.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM LOCATION(locn int, zip int, rgn_cd int, clim_zn_nbr int, tot_area_sq_ft int, sell_area_sq_ft int, avghhi int, supertargetdistance double, supertargetdrivetime double, targetdistance double, targetdrivetime double, walmartdistance double, walmartdrivetime double, walmartsupercenterdistance double, walmartsupercenterdrivetime double)
FROM FILE './datasets/retailer/Location.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM ITEM(ksn int, subcategory int, category int, categoryCluster int, prize double) 
FROM FILE './datasets/retailer/Item.tbl' LINE DELIMITED CSV(delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int]>](locn) *
    [lift<1>: RingFactorizedRelation<[1,int]>](ksn) *
    [lift<14>: RingFactorizedRelation<[14,int]>](zip) *
    [lift<3>: RingFactorizedRelation<[3,int]>](category)
)
FROM INVENTORY NATURAL JOIN LOCATION NATURAL JOIN ITEM ;
