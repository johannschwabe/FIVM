IMPORT DTREE FROM FILE 'tpch_3-Q1a.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE
STREAM lineitem (
        orderkey         INT,
        partkey          INT,
        suppkey          INT,
        l_linenumber     INT,
        l_quantity       DECIMAL,
        l_extendedprice  DECIMAL,
        l_discount       DECIMAL,
        l_tax            DECIMAL,
        l_returnflag     CHAR(1),
        l_linestatus     CHAR(1),
        l_shipdate       VCHAR(10),
        l_commitdate     VCHAR(10),
        l_receiptdate    VCHAR(10),
        l_shipinstruct   CHAR(25),
        l_shipmode       CHAR(10),
        l_comment        VARCHAR(44)
    )
  FROM FILE './datasets/tpch10/lineitem.csv'
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
  FROM FILE './datasets/tpch10/part.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE
STREAM supplier (
        suppkey        INT,
        s_name         VARCHAR(25),
        s_address         CHAR(25),
        s_nationkey        INT,
        s_phone         CHAR(15),
        s_acctbal         DECIMAL,
        s_comment         VARCHAR(101)
    )
  FROM FILE './datasets/tpch10/part.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE
STREAM partsupp (
        partkey         INT,
        suppkey         INT,
        ps_availqty     INT,
        ps_supplycost   DECIMAL,
        ps_comment      VARCHAR(199)
    )
  FROM FILE './datasets/tpch10/partsupp.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
           [lift<0>: RingFactorizedRelation<[0, INT]>](partkey) *
    [lift<1>: RingFactorizedRelation<[1, INT]>](suppkey) *
    [lift<2>: RingFactorizedRelation<[2, DECIMAL]>](l_quantity) *
    [lift<16>: RingFactorizedRelation<[16, INT]>](ps_availqty) *
    [lift<19>: RingFactorizedRelation<[19, VARCHAR(55)]>](p_name) *
    [lift<27>: RingFactorizedRelation<[27, VARCHAR(25)]>](s_name)
)
FROM lineitem
         NATURAL JOIN part
         NATURAL JOIN supplier
         NATURAL JOIN partsupp;

