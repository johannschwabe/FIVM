IMPORT DTREE FROM FILE 'tpch_1-Q2.txt';

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
        l_shipdate       CHAR(10),
        l_commitdate     CHAR(10),
        l_receiptdate    CHAR(10),
        l_shipinstruct   CHAR(25),
        l_shipmode       CHAR(10),
        l_comment        VARCHAR(44)
    )
  FROM FILE './datasets/tpch_unordered1/lineitem.csv'
  LINE DELIMITED CSV (delimiter := '|');


CREATE STREAM orders (
        orderkey         INT,
        o_custkey        INT,
        o_orderstatus    CHAR(1),
        o_totalprice     DECIMAL,
        o_orderdate      CHAR(10),
        o_orderpriority  CHAR(15),
        o_clerk          CHAR(15),
        o_shippriority   INT,
        o_comment        VARCHAR(79)
    )
  FROM FILE './datasets/tpch_unordered1/orders.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](orderkey) *
    [lift<1>: RingFactorizedRelation<[1, INT,INT,DECIMAL]>](partkey,suppkey, l_quantity) *
    [lift<16>: RingFactorizedRelation<[16, DECIMAL]>](o_totalprice)
)

FROM lineitem
NATURAL JOIN orders
