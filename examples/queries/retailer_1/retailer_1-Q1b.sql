IMPORT DTREE FROM FILE 'retailer_1-Q1b.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM Inventory(locn int, dateid int, ksn int, inventoryunits int)
FROM FILE './datasets/retailer/Inventory.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM Location(locn int, zip int, rgn_cd int, clim_zn_nbr int, tot_area_sq_ft int, sell_area_sq_ft int, avghhi int, supertargetdistance double, supertargetdrivetime double, targetdistance double, targetdrivetime double, walmartdistance double, walmartdrivetime double, walmartsupercenterdistance double, walmartsupercenterdrivetime double)
FROM FILE './datasets/retailer/Location.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM Item(ksn int, subcategory int, category int, categoryCluster int, prize double)
FROM FILE './datasets/retailer/Item.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM  Weather(locn int, dateid int, rain int, snow int, maxtemp int, mintemp int, meanwind double, thunder int)
FROM FILE './datasets/retailer/Weather.tbl' LINE DELIMITED CSV(delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int]>](ksn) *
    [lift<1>: RingFactorizedRelation<[1,int]>](locn) *
    [lift<2>: RingFactorizedRelation<[2,int]>](dateid) *
    [lift<3>: RingFactorizedRelation<[3,int]>](rain) *
    [lift<11>: RingFactorizedRelation<[11,int]>](zip) *
    [lift<26>: RingFactorizedRelation<[26,int]>](category)
)
FROM Inventory NATURAL JOIN Location NATURAL JOIN Item NATURAL JOIN  Weather;
