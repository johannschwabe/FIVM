IMPORT DTREE FROM FILE 'retailer.txt';

CREATE TYPE RingCofactorGeneral
FROM FILE 'ring/ring_cofactor_general.hpp'
WITH PARAMETER SCHEMA (dynamic_min, dynamic_sum, dynamic_sum);

CREATE STREAM Inventory(locn int, dateid int, ksn int, inventoryunits int)
FROM FILE './datasets/retailer_unordered/Inventory.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM Location(locn int, zip int, rgn_cd int, clim_zn_nbr int, tot_area_sq_ft int, sell_area_sq_ft int, avghhi int, supertargetdistance double, supertargetdrivetime double, targetdistance double, targetdrivetime double, walmartdistance double, walmartdrivetime double, walmartsupercenterdistance double, walmartsupercenterdrivetime double)
FROM FILE './datasets/retailer_unordered/Location.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM  Census(zip int, population int, white int, asian int, pacific int, blackafrican int, medianage double, occupiedhouseunits int, houseunits int, families int, households int, husbwife int, males int, females int, householdschildren int, hispanic int)
FROM FILE './datasets/retailer_unordered/Census.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM Item(ksn int, subcategory int, category int, categoryCluster int, prize double)
FROM FILE './datasets/retailer_unordered/Item.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM  Weather(locn int, dateid int, rain int, snow int, maxtemp int, mintemp int, meanwind double, thunder int)
FROM FILE './datasets/retailer_unordered/Weather.tbl' LINE DELIMITED CSV(delimiter := '|');

SELECT SUM(
    [liftcont<0>: RingCofactorGeneral<0,1,0>](inventoryunits) *
    [liftcont<1>: RingCofactorGeneral<1,4,0>](subcategory, category, categoryCluster, prize) *
    [liftcont<5>: RingCofactorGeneral<5,6,0>](rain, snow, maxtemp, mintemp, meanwind, thunder) *
    [liftcont<11>: RingCofactorGeneral<11,13,0>](rgn_cd, clim_zn_nbr, tot_area_sq_ft, sell_area_sq_ft, avghhi, supertargetdistance, supertargetdrivetime, targetdistance, targetdrivetime, walmartdistance, walmartdrivetime, walmartsupercenterdistance, walmartsupercenterdrivetime) *
    [liftcont<24>: RingCofactorGeneral<24,15,0>](population, white, asian, pacific, blackafrican, medianage, occupiedhouseunits, houseunits, families, households, husbwife, males, females, householdschildren, hispanic)
)
FROM Inventory NATURAL JOIN Location NATURAL JOIN  Census NATURAL JOIN Item NATURAL JOIN  Weather;
