IMPORT DTREE FROM FILE 'retailer_1Q2.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM INVENTORY(locn int, dateid int, ksn int, inventoryunits int)
FROM FILE './datasets/retailer/Inventory.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM LOCATION(locn int, zip int, rgn_cd int, clim_zn_nbr int, tot_area_sq_ft int, sell_area_sq_ft int, avghhi int, supertargetdistance double, supertargetdrivetime double, targetdistance double, targetdrivetime double, walmartdistance double, walmartdrivetime double, walmartsupercenterdistance double, walmartsupercenterdrivetime double)
FROM FILE './datasets/retailer/Location.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM WEATHER(locn int, dateid int, rain int, snow byte, maxtemp int, mintemp int, meanwind double, thunder byte)
FROM FILE './datasets/retailer/Weather.tbl' LINE DELIMITED CSV(delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int]>](locn) *
    [lift<1>: RingFactorizedRelation<[1,int]>](dateid) *
    [lift<2>: RingFactorizedRelation<[2,int]>](ksn) *
    [lift<4>: RingFactorizedRelation<[4,int,int]>](rain, maxtemp) *
    [lift<10>: RingFactorizedRelation<[10,int]>](zip)
)
FROM INVENTORY NATURAL JOIN LOCATION NATURAL JOIN WEATHER;
