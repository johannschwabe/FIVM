IMPORT DTREE FROM FILE 'retailer_2-Q2.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM Location(locn int, zip int, rgn_cd int, clim_zn_nbr int, tot_area_sq_ft int, sell_area_sq_ft int, avghhi int, supertargetdistance double, supertargetdrivetime double, targetdistance double, targetdrivetime double, walmartdistance double, walmartdrivetime double, walmartsupercenterdistance double, walmartsupercenterdrivetime double)
FROM FILE './datasets/retailer_unordered/Location.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM  Weather(locn int, dateid int, rain int, snow int, maxtemp int, mintemp int, meanwind double, thunder int)
FROM FILE './datasets/retailer_unordered/Weather.tbl' LINE DELIMITED CSV(delimiter := '|');


SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0,int]>](locn) *
    [lift<1>: RingFactorizedRelation<[1,int,int,int,int,int,int,double,double,double,double,double,double,double,double]>](zip,rgn_cd, clim_zn_nbr, tot_area_sq_ft, sell_area_sq_ft, avghhi, supertargetdistance, supertargetdrivetime, targetdistance, targetdrivetime, walmartdistance, walmartdrivetime, walmartsupercenterdistance, walmartsupercenterdrivetime) *
    [lift<15>: RingFactorizedRelation<[15,int,int,int,int,int,double,int]>](dateid,rain, snow, maxtemp, mintemp, meanwind, thunder)
)
FROM Location NATURAL JOIN  Weather;
