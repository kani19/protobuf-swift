// Protocol Buffers for Swift
//
// Copyright 2014 Alexey Khohklov(AlexeyXo).
// Copyright 2008 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "swift_generator.h"

#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/common.h>

#include "swift_file.h"
#include "swift_helpers.h"

namespace google { namespace protobuf { namespace compiler { namespace swift {
    SwiftGenerator::SwiftGenerator() {
    }


    SwiftGenerator::~SwiftGenerator() {
    }


    bool SwiftGenerator::Generate(const FileDescriptor* file,
                                  const string& parameter,
                                  GeneratorContext* generator_context,
                                  string* error) const {
        
        

        static map<string,string> packages;
        
        vector< pair<string, string> > options;
        ParseGeneratorParameter(parameter, &options);

        for (int i = 0; i < options.size(); i++) {
            *error = "Unknown generator option: " + options[i].first;
            return false;
        }

        FileGenerator file_generator(file);

        string filepath = FilePath(file);
        {
           
            string package_name;
           
            if (file->package() != "") {
                package_name = FullName(file);
            }
            
            vector<string> tokens = FullNameSplit(file);
            
            scoped_ptr<io::ZeroCopyOutputStream> output(generator_context->Open(package_name + UnderscoresToCapitalizedCamelCase(UnderscoresToCapitalizedCamelCase(filepath)) + ".proto.swift"));
            io::Printer printer(output.get(), '$');
            
            
            printer.Print("// Generated by the Protocol Buffers $version$ compiler.  DO NOT EDIT!\n"
                          "// Source file \"$filePath$\"\n",
                          "filePath", FileNameDescription(file),
                          "version",
                          internal::VersionString(GOOGLE_PROTOBUF_VERSION));
            printer.Print("// Protobuf-swift version: $version$\n", "version", SWIFT_PROTOBUF_VERSION);
            
            if (file->syntax() == FileDescriptor::SYNTAX_PROTO2) {
                printer.Print("// Syntax \"Proto2\"\n\n");
            } else {
                printer.Print("// Syntax \"Proto3\"\n\n");
            }
            
            printer.Print("import Foundation\n");
            
            if (isCompileForFramework(file)) {
                printer.Print("import ProtocolBuffers\n\n");
            }
            printer.Print("\n");
            
            string lastPackage = "";
            bool isPreviousExtensions = false;
            bool extensionsOnly = true;
            int countBraces = 0;
            
                for (int i = 0; i < tokens.size(); i++)
                {
                    if ((packages.find((lastPackage + "." + tokens[i])) == packages.end() && !IsBootstrapPackage(tokens[i])) || (IsBootstrapPackage(tokens[i]) && IsBootstrapFile(file))) {
                    
                        if (isPreviousExtensions) {
                            printer.Print("{ ");
                            countBraces++;
                        }
                        printer.Print("$acontrol$ struct $package$ { ",
                                      "acontrol", GetAccessControlType(file),
                                      "package", tokens[i]);
                        lastPackage = lastPackage + "." + tokens[i];
                        packages[lastPackage] = "generated";
                        isPreviousExtensions = false;
                        extensionsOnly = false;
                        countBraces++;
                    }
                    else
                    {
                        if (!isPreviousExtensions) {
                            printer.Print("$acontrol$ extension $package$",
                                          "acontrol", GetAccessControlType(file),
                                          "package", tokens[i]);
                            isPreviousExtensions = true;
                            lastPackage = lastPackage + "." + tokens[i];
                        }
                        else
                        {
                            printer.Print(".$package$",
                                          "package", tokens[i]);
                            isPreviousExtensions = true;
                            lastPackage = lastPackage + "." + tokens[i];
                        }
                        
                    }
                    
                    if (i == tokens.size() -1 ) {
                        if (extensionsOnly) {
                            printer.Print("{}");
                        }
                        
                        for (int i = 0; i < countBraces; i++)
                        {
                            printer.Print("}");
                        }
                        printer.Print("\n\n");
                        
                    }
                    
                }
            
            file_generator.GenerateSource(&printer);
        }

        return true;
    }

}  // namespace swift
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
