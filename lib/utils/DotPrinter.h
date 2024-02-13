#pragma once

#include <iostream>
#include <ranges>
#include <string_view>

class DotPrinter {
   using string_view = std::string_view;
   using ostream = std::ostream;

   struct Sanitize {
      string_view str;
      Sanitize(string_view str) : str(str) {}
   };

   friend ostream& operator<<(ostream& os, Sanitize s);

public:
   DotPrinter(ostream& os) : os(os) {}

   /// @brief Prints a table row with a single column
   void print_table_single_row(string_view cell_text) {
      print_html_start("tr");
      print_html_start("td", {"colspan", "2", "height", "35"});
      os << Sanitize{cell_text};
      print_html_end("td");
      print_html_end("tr");
   }

   /// @brief Prints a table row with 2 columns
   void print_table_double_row(string_view cell1_text, string_view cell2_text) {
      print_html_start("tr");
      print_html_start("td");
      os << Sanitize{cell1_text};
      print_html_end("td");
      print_html_start("td", {"height", "35", "width", "35"});
      os << Sanitize{cell2_text};
      print_html_end("td");
      print_html_end("tr");
   }

   /// @brief Starts a DOT label that is also an HTML table
   void start_tlabel(int id, std::initializer_list<string_view> attrs = {}) {
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
            "cellpadding", "5",
            "margin", "0"
         }
      );
      // clang-format on
   }

   /// @brief Ends a DOT label that is also an HTML table
   void end_tlabel() {
      print_html_end("table");
      indent_--;
      indent() << ">];\n";
   }

   /// @brief Starts a DOT label
   void start_label(int id, std::initializer_list<string_view> attrs = {}) {
      indent() << "node" << id << " [shape=rect";
      print_attr_list(attrs, false);
      os << " label=<";
   }

   /// @brief Ends a DOT label
   void end_label() { os << ">];\n"; }

   /// @brief Prints a DOT label
   void print_label(int id,
                    string_view label,
                    std::initializer_list<string_view> attrs = {}) {
      start_label(id, attrs);
      sanitize(label);
      end_label();
   }

   /// @brief Prints the DOT subgraph { given id
   void start_subgraph(int id, string_view label = "") {
      indent() << "subgraph cluster_" << id << " {\n";
      indent_++;
      if(!label.empty()) {
         indent() << "label=<" << Sanitize{label} << ">;\n";
      }
   }

   /// @brief Prints the remainder of the DOT subgraph
   void end_subgraph() {
      indent_--;
      indent() << "}\n";
   }

   /// @brief Prints the DOT graph {
   void start_graph() {
      os << "digraph G {\n";
      indent_++;
   }

   /// @brief Prints the remainder of the DOT graph
   void end_graph() {
      indent_--;
      os << "}\n";
   }

   /// @brief Prints a DOT connection between 2 nodes
   void print_connection(int from, int to) {
      indent() << "node" << from << " -> node" << to << ";\n";
   }

   /// @brief Allocates a new unique id
   int id() { return id_++; }

   /// @brief Print a santized string as a label value
   void sanitize(string_view str) { os << Sanitize{str}; }

private:
   ostream& indent() {
      for(int i = 0; i < indent_; i++) os << "  ";
      return os;
   }

   void print_attr_list(std::initializer_list<string_view> attrs, bool quote) {
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
                         std::initializer_list<string_view> attrs = {}) {
      bool newline = true;
      if(tag == "td") newline = false;
      indent() << "<" << tag;
      print_attr_list(attrs, true);
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
};
