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

import fdbresearch.parsing.{DTreeParser, SQLParser}
import fdbresearch.util.Logger
import java.io.File

object Main extends App {

  case class Config(inputSQL: String = "",
                    batchUpdates: Boolean = true,
                    outputM3: Option[String] = None)

  val parser = new scopt.OptionParser[Config]("sbt run") {
    override def showUsageOnError = true

    head("F-IVM", "1.0")

    arg[String]("<SQL file>").required().action((x, c) =>
      c.copy(inputSQL = x)).text("Input SQL file")

    opt[String]('o', "output").valueName("<M3 file>").action((x, c) =>
      c.copy(outputM3 = Some(x))).text("Output M3 file")

    opt[Unit]("batch").action((_, c) =>
      c.copy(batchUpdates = true)).text("Generate batch update triggers")

    opt[Unit]("single").action((_, c) =>
      c.copy(batchUpdates = false)).text("Generate single-tuple update triggers")

    help("help").text("prints this usage text")
  }

  def parseSQL(f: File) = {
    if (!f.exists) {
      sys.error("Input SQL file does not exist: " + f.getAbsolutePath)
    }
    new SQLParser().apply(scala.io.Source.fromFile(f).mkString)
  }

  def parseDTree(f: File) = {
    if (!f.exists) {
      sys.error("Input DTree file does not exist: " + f.getAbsoluteFile)
    }
    new DTreeParser().apply(scala.io.Source.fromFile(f).mkString)
  }

  parser.parse(args, Config()) match {
    case Some(config) =>
      val sqlFile = new File(config.inputSQL)
      val sql = parseSQL(sqlFile)
      Logger.instance.debug("SQL AST: " + sql.toString)

      val dtreeFile = new File(sqlFile.getParentFile.getAbsolutePath, sql.dtree.file.path)
      val dtree = parseDTree(dtreeFile)

      val (output, configFile) = new Driver().compile(sql, dtree, config.batchUpdates)
      config.outputM3 match {
        case Some(file) => {
          new java.io.PrintWriter(file) {
            write(output);
            close()
          }
          val split_file_path = file.split("/")
          val file_name = split_file_path(3).split("_").init.mkString("_")
          val config_file_path = split_file_path(0) + "/config/" + split_file_path(2) + "/" + file_name + ".txt"
          new java.io.PrintWriter(config_file_path) {
            val splitted = dtreeFile.getName.split('.')(0)
            val fixed_config = splitted +"\n"+ configFile.replace("FIVMQUERY", splitted.split('-')(1))
            write(fixed_config);
            close()
          }
        }
        case None => println(output)
      }

    case None => // bad arguments, error message displayed
  }
}