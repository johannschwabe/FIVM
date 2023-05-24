//===----------------------------------------------------------------------===//
//
// Factorized IVM (F-IVM)
//
// https://fdbresearch.github.io/
//
// Copyright (c) 2018-2019, FDB Research Group, University of Oxford
// 
//===----------------------------------------------------------------------===//
package fdbresearch

import fdbresearch.tree.{DTreeNode, DTreeRelation, Tree, View, ViewTree}
import fdbresearch.core.{SQL, SQLToM3Compiler, Source}
import fdbresearch.parsing.M3Parser
import fdbresearch.util.Logger

class Driver {

  import fdbresearch.tree.DTree._

  // TODO: allow definitions of unused streams and tables

  /**
    * Check if all SQL sources have consistent schemas with DTree relations
    */
  private def checkSchemas(sqlSources: List[Source], relations: List[DTreeRelation]): Unit = {
    val rm = relations.map { r =>
      r.name -> r.keys.map(v => (v.name, v.tp)).toSet
    }.toMap
    val diff = sqlSources.flatMap { s =>
      val f1 = s.schema.fields.toSet
      val f2 = rm(s.schema.name)
      f1.diff(f2).union(f2.diff(f1))
    }
    assert(diff.isEmpty, "Inconsistent schemas in SQL and DTree files:\n" + diff.mkString("\n"))
  }

  /**
    * Resolve missing types in SQL system
    */
  private def resolveTypes(s: SQL.System): SQL.System = {
    val vm = s.sources.flatMap(_.schema.fields.map(x => x._1 -> x._2)).toMap
    s.replace {
      case SQL.Field(n, t, tp) =>
        assert(tp == null || tp == vm(n))
        SQL.Field(n, t, vm(n))
    }.asInstanceOf[SQL.System]
  }

  def findVars(a: Tree[View],
               keyMap: scala.collection.mutable.Map[String, String],
               payloadViews: scala.collection.mutable.MutableList[String]): scala.collection.mutable.Map[String, List[String]] = {
    val res = scala.collection.mutable.Map[String, List[String]]()
    //Logger.instance.info(a.node.name)
    //Logger.instance.info(a.node.terms.toString)
    if(a.node.terms.length > 0) {
      res += (a.node.name -> a.node.terms(0).schema._1.map(i => {
        i._1
      }))
    } else {
      res += (a.node.name -> List.empty[String])
    }
    var completeCover = true
    a.children.foreach(child => {
      if(keyMap.contains(child.node.name)){
        var child_res = findVars(child, keyMap, payloadViews)
//        Logger.instance.info("name: " + child.node.name)
//        Logger.instance.info("res.values.toSet: " + res.values.flatten.toSet.toString())
//        Logger.instance.info("chi.values.toSet: " + child_res.values.flatten.toSet.toString())
        if(!child_res.values.flatten.toSet.subsetOf(res.values.flatten.toSet) && !res.values.flatten.toSet.equals(child_res.values.flatten.toSet)){
//          Logger.instance.info("subset")
          child_res.foreach(view => res += (view._1 -> view._2))
          completeCover = false
        }
      }
    })
    if(completeCover){
      payloadViews += a.node.name.substring(2)
    }
    res
  }

  def compile(sql: SQL.System, dtree: Tree[DTreeNode], batchUpdates: Boolean): (String, String) = {

    checkSchemas(sql.sources, dtree.getRelations)

    Logger.instance.debug("CHECK SCHEMAS: OK")

    val typedSQL = resolveTypes(sql)
    val (sumFn, _, whCond, gb) = SQLToM3Compiler.compile(typedSQL)

    Logger.instance.debug("BUILDING VIEW TREE:")

    val viewtree = ViewTree(dtree, gb.toSet, sumFn, whCond)

    Logger.instance.debug("\n\nVIEW TREE:\n" + viewtree)

    val cg = new CodeGenerator(viewtree, sql.typeDefs, sql.sources, batchUpdates)

    var config = ""
    config = config + sql.sources(0).in.toString.split("/")(2)
    config = config + "\n"
    config = config + "tbl\n"
    config = config + "FIVM\n"
    config = config + "1000\n"

    val keyMap = scala.collection.mutable.Map[String, String]()
    val viewOrder = scala.collection.mutable.MutableList[String]()
    val payloadViews = scala.collection.mutable.MutableList[String]()
    cg.generateQueries.foreach(t => {
      viewOrder += t.name
      var ovars = t.expr.ovars.map(ovar => {
        ovar._1
      })
      keyMap += (t.name -> ovars.mkString(","))
    })
    config = config + "FIVMQUERY|" + viewOrder.length.toString + "|0\n"
    val all_relations = dtree.getRelations.map(i => i.name.toString).mkString("|")
    config = config + all_relations + "\n\n"
    var payloadMap = findVars(cg.getTree, keyMap, payloadViews)
    viewOrder.foreach(view => {
      var payloadView = if (payloadViews.contains(view.substring(2))) "1" else "0"
      config = config + view.substring(2) + "|" + keyMap(view) + "|" + payloadMap(view).mkString(",") + "|" + payloadView + "\n"
    })
    //Logger.instance.info(payloadMap.mkString("\n"))


    //cg.getTree.map2(t => Logger.instance.info(t.node.freeVars.mkString))

    val m3 = cg.generateM3
    Logger.instance.debug("\n\nORIGINAL M3\n" + m3)

    val optM3 = Optimizer.optimize(m3)
    Logger.instance.debug("\n\nOPTIMIZED M3\n" + optM3)

    // test that the output can be parsed by the M3 parser
    val checkedM3 = new M3Parser().apply(optM3.toString)
    Logger.instance.debug("M3 SYNTAX CHECKED")

    (checkedM3.toString, config)
  }
}
