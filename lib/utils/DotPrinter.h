#pragma once

#include <iostream>
#include <ranges>
#include <string_view>

/// @brief A class to help print DOT graphs!
class DotPrinter {
   using string_view = std::string_view;
   using ostream = std::ostream;
   using string_list = std::initializer_list<string_view>;

   struct Sanitize {
      string_view str;
      Sanitize(string_view str) : str(str) {}
   };

   friend ostream& operator<<(ostream& os, Sanitize s);

public:
   /// @brief Constructor takes in the output stream to print into
   DotPrinter(ostream& os) : os(os), min_height_{"0"} {}

   /// @brief Constructor takes in the output stream to print into
   DotPrinter(ostream& os, string_view min_height)
         : os(os), min_height_{min_height} {}

   /// @brief Prints a table row with a single column
   void printTableSingleRow(string_view cell_text,
                            string_list cell_attrs = {}) {
      print_html_start("tr");
      print_html_start(
            "td", {"colspan", "2", "height", min_height_}, cell_attrs);
      os << Sanitize{cell_text};
      print_html_end("td");
      print_html_end("tr");
   }

   /// @brief Prints a table row with 2 columns
   void printTableDoubleRow(string_view cell1_text,
                            string_view cell2_text,
                            string_list cell1_attrs = {},
                            string_list cell2_attrs = {}) {
      print_html_start("tr");
      print_html_start("td", cell1_attrs);
      os << Sanitize{cell1_text};
      print_html_end("td");
      print_html_start(
            "td", {"height", min_height_, "width", min_height_}, cell2_attrs);
      os << Sanitize{cell2_text};
      print_html_end("td");
      print_html_end("tr");
   }

   /// @brief Starts a DOT label that is also an HTML table
   void startTLabel(int id,
                    string_list attrs = {},
                    string_view cellpadding = "0") {
      indent() << "node" << id << " [shape=none margin=0.01";
      print_attr_list(attrs, false);
      os << " label=<\n";
      indent_++;
      // clang-format off
      print_html_start(
         "table",
         {
            "border", "0",
            "cellborder", "1",
            "cellspacing", "0",
            "cellpadding", cellpadding,
            "margin", "0"
         }
      );
      // clang-format on
   }

   /// @brief Ends a DOT label that is also an HTML table
   void endTLabel() {
      print_html_end("table");
      indent_--;
      indent() << ">];\n";
   }

   /// @brief Starts a DOT label
   void startLabel(int id, string_list attrs = {}) {
      indent() << "node" << id << " [shape=rect";
      print_attr_list(attrs, false);
      os << " label=<";
   }

   /// @brief Ends a DOT label
   void endLabel() { os << ">];\n"; }

   /// @brief Prints a DOT label
   void printLabel(int id, string_view label, string_list attrs = {}) {
      startLabel(id, attrs);
      sanitize(label);
      endLabel();
   }

   /// @brief Prints the DOT subgraph { given id
   void startSubgraph(int id, string_view label = "") {
      indent() << "subgraph cluster_" << id << " {\n";
      indent_++;
      if(!label.empty()) {
         indent() << "label=<" << Sanitize{label} << ">;\n";
      }
   }

   /// @brief Prints the remainder of the DOT subgraph
   void endSubgraph() {
      indent_--;
      indent() << "}\n";
   }

   /// @brief Prints the DOT graph {
   void startGraph() {
      os << "digraph G {\n";
      indent_++;
   }

   /// @brief Prints the remainder of the DOT graph
   void endGraph() {
      indent_--;
      os << "}\n";
   }

   /// @brief Prints a DOT connection between 2 nodes
   void printConnection(int from, int to) {
      indent() << "node" << from << " -> node" << to << ";\n";
   }

   /// @brief Prints a DOT connection between 2 nodes
   /// @param from The node from which the connection starts
   /// @param port1 The port of the from node
   /// @param to The node to which the connection ends
   /// @param lhead_cluster If we should connect to a subgraph, the id of it
   void printConnection(int from,
                        string_view port,
                        int to,
                        int lhead_cluster = -1) {
      indent() << "node" << from << port << " -> node" << to;
      if(lhead_cluster != -1) {
         os << " [lhead=cluster_" << lhead_cluster << "]";
      }
      os << "\n";
   }

   /// @brief Allocates a new unique id
   int id() { return id_++; }

   /// @brief Print a santized string as a label value
   void sanitize(string_view str) { os << Sanitize{str}; }

   /// @brief Print a single line of indented text
   void print(string_view str) { indent() << str << "\n"; }

private:
   ostream& indent() {
      for(int i = 0; i < indent_; i++) os << "  ";
      return os;
   }

   void print_attr_list(string_list attrs, bool quote) {
      bool isKey = true;
      for(auto attr : attrs) {
         if(isKey) {
            os << " " << attr << "=";
         } else {
            if(quote) os << "\"";
            os << attr;
            if(quote) os << "\"";
         }
         isKey = !isKey;
      }
   }

   void print_html_start(string_view tag,
                         string_list attrs = {},
                         string_list attrs2 = {}) {
      bool newline = true;
      if(tag == "td") newline = false;
      indent() << "<" << tag;
      print_attr_list(attrs, true);
      print_attr_list(attrs2, true);
      os << ">";
      if(newline) {
         os << "\n";
         indent_++;
      }
   }

   void print_html_end(string_view tag) {
      bool newline = true;
      if(tag == "td") newline = false;
      if(newline) {
         indent_--;
         indent();
      }
      os << "</" << tag << ">\n";
   }

private:
   ostream& os;
   int indent_ = 0;
   int id_ = 0;
   string_view min_height_;
};
