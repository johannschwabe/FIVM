IMPORT DTREE FROM FILE 'tpch_5-Q3.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE
STREAM part (
        partkey        INT,
        p_name         VARCHAR(55),
        p_mfgr         CHAR(25),
        p_brand        CHAR(10),
        p_type         VARCHAR(25),
        p_size         INT,
        p_container    CHAR(10),
        p_retailprice  DECIMAL,
        p_comment      VARCHAR(23)
    )
  FROM FILE './datasets/tpch_unordered1/part.csv'
  LINE DELIMITED CSV (delimiter := '|');


CREATE STREAM partsupp (
        partkey         INT,
        suppkey         INT,
        ps_availqty     INT,
        ps_supplycost   DECIMAL,
        ps_comment      VARCHAR(199)
    )
  FROM FILE './datasets/tpch_unordered1/partsupp.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](partkey) *
    [lift<1>: RingFactorizedRelation<[1, INT, INT]>](suppkey, ps_availqty) *
    [lift<3>: RingFactorizedRelation<[3, CHAR(25)]>](p_name)
   )
FROM partsupp NATURAL JOIN part;