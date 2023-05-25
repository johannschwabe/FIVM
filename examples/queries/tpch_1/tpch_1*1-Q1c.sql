IMPORT DTREE FROM FILE 'tpch_1-Q1c.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE
STREAM q2 (
    orderkey             INT,
    partkey              INT,
    suppkey              INT,
    l_quantity           DECIMAL,
    o_totalprice        DECIMAL
    )
  FROM FILE './datasets/tpch1/q2.csv'
  LINE DELIMITED CSV (delimiter := '|');

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
  FROM FILE './datasets/tpch1/part.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE
STREAM partsupp (
        partkey         INT,
        suppkey         INT,
        ps_availqty     INT,
        ps_supplycost   DECIMAL,
        ps_comment      VARCHAR(199)
    )
  FROM FILE './datasets/tpch1/partsupp.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](partkey) *
    [lift<1>: RingFactorizedRelation<[1, INT]>](suppkey) *
    [lift<2>: RingFactorizedRelation<[2, INT,DECIMAL,DECIMAL]>](orderkey,l_quantity,o_totalprice) *
    [lift<16>: RingFactorizedRelation<[16, INT]>](ps_availqty) *
    [lift<19>: RingFactorizedRelation<[19, VARCHAR(55)]>](p_name)
)

FROM q2 NATURAL JOIN part
NATURAL JOIN partsupp;
