IMPORT DTREE FROM FILE 'tpch_4-Q2.txt';

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
        l_returnflag     VARCHAR(1),
        l_linestatus     VARCHAR(1),
        l_shipdate       VARCHAR(10),
        l_commitdate     VARCHAR(10),
        l_receiptdate    VARCHAR(10),
        l_shipinstruct   CHAR(25),
        l_shipmode       CHAR(10),
        l_comment        VARCHAR(44)
    )
  FROM FILE './datasets/jcch_unordered1/lineitem.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE
STREAM supplier (
        suppkey        INT,
        s_name         VARCHAR(25),
        s_address         CHAR(25),
        nationkey        INT,
        s_phone         CHAR(15),
        s_acctbal         DECIMAL,
        s_comment         VARCHAR(101)
    )
  FROM FILE './datasets/jcch_unordered1/part.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE
STREAM partsupp (
        partkey         INT,
        suppkey         INT,
        ps_availqty     INT,
        ps_supplycost   DECIMAL,
        ps_comment      VARCHAR(199)
    )
  FROM FILE './datasets/jcch_unordered1/partsupp.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](suppkey) *
    [lift<1>: RingFactorizedRelation<[1, INT]>](partkey) *
    [lift<2>: RingFactorizedRelation<[2, DECIMAL,int]>](l_quantity, orderkey) *
    [lift<17>: RingFactorizedRelation<[17, INT, DECIMAL]>](ps_availqty, ps_supplycost) *
    [lift<20>: RingFactorizedRelation<[20, INT, VARCHAR(25)]>](nationkey,s_name)
)
FROM lineitem
         NATURAL JOIN supplier
         NATURAL JOIN partsupp;
