IMPORT DTREE FROM FILE 'retailer.txt';

CREATE TYPE RingCofactorMixed
FROM FILE 'ring/ring_cofactor_degree1_categorical.hpp'
WITH PARAMETER SCHEMA (dynamic_min, dynamic_sum, dynamic_sum);

CREATE STREAM INVENTORY(locn long, dateid long, ksn double, inventoryunits double)
FROM FILE './datasets/sears/Inventory.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM LOCATION(locn long, zip long, rgn_cd double, clim_zn_nbr double, tot_area_sq_ft double, sell_area_sq_ft double, avghhi double, supertargetdistance double, supertargetdrivetime double, targetdistance double, targetdrivetime double, walmartdistance double, walmartdrivetime double, walmartsupercenterdistance double, walmartsupercenterdrivetime double)
FROM FILE './datasets/sears/Location.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM CENSUS(zip long, population long, white long, asian long, pacific long, blackafrican long, medianage double, occupiedhouseunits long, houseunits long, families long, households long, husbwife long, males long, females long, householdschildren long, hispanic long)
FROM FILE './datasets/sears/Census.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM ITEM(ksn double, subcategory long, category long, categoryCluster long, prize double) 
FROM FILE './datasets/sears/Item.tbl' LINE DELIMITED CSV(delimiter := '|');

CREATE STREAM WEATHER(locn long, dateid long, rain long, snow long, maxtemp long, mintemp long, meanwind double, thunder long) 
FROM FILE './datasets/sears/Weather.tbl' LINE DELIMITED CSV(delimiter := '|');

SELECT SUM(
    [liftCont<0>: RingCofactorMixed<0,1,0>](inventoryunits) *
    [liftCont<1>: RingCofactorMixed<1,1,0>](prize) *
    [liftCat<2>: RingCofactorMixed<2,0,3>](subcategory, category, categoryCluster) *
    [liftCont<5>: RingCofactorMixed<5,3,0>](maxtemp, mintemp, meanwind) *
    [liftCat<8>: RingCofactorMixed<8,0,3>](rain, snow, thunder) *
    [liftCont<11>: RingCofactorMixed<11,13,0>](rgn_cd, clim_zn_nbr, tot_area_sq_ft, sell_area_sq_ft, avghhi, supertargetdistance, supertargetdrivetime, targetdistance, targetdrivetime, walmartdistance, walmartdrivetime, walmartsupercenterdistance, walmartsupercenterdrivetime) *
    [liftCont<24>: RingCofactorMixed<24,15,0>](population, white, asian, pacific, blackafrican, medianage, occupiedhouseunits, houseunits, families, households, husbwife, males, females, householdschildren, hispanic)
)
FROM INVENTORY NATURAL JOIN LOCATION NATURAL JOIN CENSUS NATURAL JOIN ITEM NATURAL JOIN WEATHER;