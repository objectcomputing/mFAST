Introduction
============

mFast is an open source C++ implementation of FAST decoder. The FAST protocol (FIX Adapted for STreaming) is a technology standard developed
by FIX Protocol Ltd., specifically aimed at optimizing data representation on the network. It is used to support high-throughput, low latency
data communications between financial institutions.

mFAST was designed from the ground to provide an easy to use and yet efficient FAST decoder. Based on my initial experience to
QuickFAST[www.quickfast.org], I saw there were some room for improvement. However,  the
improvement couldn't be achieved without an architectural redesign. Therefore, I took it as a personal project and
worked on it mostly at night and weekends. After two and a half months of development, I am proudly to present this work.

Compared to QuickFAST, mFAST offers the following features:

An easier to use application type system
-----------------------------------------
The FAST encoding is performed with respect to a control structure called a template. A template
controls the encoding of a portion of the stream by specifying the order and structure of fields, their field operators and
the binary encoding representations to use. A FAST template is defined with a concrete syntax using XML format. One of the
most distinctive advance of mFAST over QuickFast is its ability to generate C++ application types using the XML specification.
If you are familiar with CORBA or DDS, this process is tantamount to the proxy/stub code generation using IDL compiler.


Lower external dependencies
---------------------------------------
Beside C++ standard library, the core mFAST decoder uses header only boost library to provide platform
portability. In addition, it uses TinyXML2 for XML parsing. However, TinyXML2 has only one source and one header file and it is
statically linked to mFAST applications.

Smaller memory footprint
-----------------------------------

Better runtime efficiency
----------------------------


Getting Start
=================

Prerequisite
------------------------
mFAST is dependent on the following tools/libraries

* BOOST 1.53 or higher [http://www.boost.org]
* CMake 2.8.8 or higher [http://cmake.org]
* tinyXML2 [http://www.grinninglizard.com/tinyxml2] (setup as a submodule in the git repository)

Building the code
------------------------

mFAST uses CMake as its build system. On Unix systems, you can simply following the instruction below 
to compile the code.
    
    $ cd mfast
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make 
 
This uses the CMake out-of-source building approach to compile the code. You can also use `make install` to install the built library and tool into the system predefined path (/usr/local). For convenience, we will refer the build directory as `$MFAST_BUILD_DIR`.
    
The build system can be configured via `-DOption=Value` syntax when invoking cmake. 
Here are a list commonly used configuration options for mFAST.

* CMAKE_BUILD_TYPE : Specify the building mode such as debug or release. Valid values are
    - None (default)
    - Debug 
    - Relase
    - RelWithDebInfo
    - MinSizeRel
* BUILD_SHARED_LIBS : whether to build mFAST as shared/dynamic linked library. Valid values are OFF and ON, where the default is OFF which builds both static and shared library. Notice that for application to link against the shared library, the preprocessor macro MAST_DYN_LINK must be defined for the application sources.
* BOOST_ROOT : specify the root directory for your boost library installation. If you have problem for the build system to find your boost installation after specifying BOOST_ROOT, use `cmake --help-module FindBoost` in the command line to get further information.
* CMAKE_INSTALL_PREFIX : specify the installation prefix for mFAST library and code generation tool.

To make sure if mFAST can work correctly on your platform, you can run the unit test program `$MFAST_BUILD_DIR/tests/mfast_test`.

On windows system with MSVC, you can download the CMake GUI tool to generate MSVC project files.
The building process and unit test program require the boost DLLs; therefore  make sure the boost DLL path is in your PATH environment variable before starting compiling mFAST.


Basics of FAST template 
----------------------------

Before we dive into the detail of mFAST APIs, we need to briefly introduce the FAST template specification to facilitate our further discussion. Conceptually, a FAST template is similar to a `struct` in C language. It contains one or more fields of primitive types, groups or sequences. 
As of FAST specification 1.1, there are only 7 primitive types: int32, uInt32, int64, uInt64, decimal, string and byteVector. A string type can be further divided into ASCII and unicode string types. A group is similar to nested struct in C which comprises an ordered set of other fields. Sequence is similar to the vector in C++ which contains a repeated group. It is also possible to nest another template inside a template, in which case the field is called `templateRef`. A `templateRef` field can have a specified name or not. A templateRef with name specified is called static templateRef whereas a templateRef without a  name specified is called dynamic templateRef. A static templateRef is treated as if the content of the referenced template is directly copied into the referencing template under the FAST encoding rule. A   dynamic templateRef field, on the other hand, can be assigned to any value of a template recognizable to the encoder/decoder at runtime. In certain sense, it is the  way of doing ANY type in FAST; although it can only used to embed other templates, not to embed other primitive types, groups or sequences.

Except for templateRef, all fields can be declared as mandatory or optional. In addition, all fields of primitive can be specified with an `instruction context` which describe how the value of a field  can be interpreted relative to its previous value of the same field. For example, a `constant` instruction means the value of the field can never change except for it can be absent when the field is optional. A field with copy instruction means the decoder should interpret the value of the field as its previous value when value is not present in the encoded stream; otherwise, the value of the field is the same with the value on the stream.


The following fragment of XML is an example of a template definition.

    <templates xmlns="http://www.fixprotocol.org/ns/template-definition" 
               templateNs="http://www.fixprotocol.org/ns/templates/sample" 
               ns="http://www.fixprotocol.org/ns/fix">
    <template name="MDRefreshSample" id='10001'>
        <string name="BeginString" id=”8”> <constant value="FIX4.4"/> </string> 
        <string name="MessageType" id=”35”> <constant value="X"/> </string> 
        <string name="ApplVerID" id="1128" presence="optional"> <copy/> </string>
        <string name="SenderCompID" id=”49”> <copy/> </string>
        <uInt32 name="MsgSeqNum" id=”34”> <increment/> </uInt32> 
        <sequence name="MDEntries">
            <length name="NoMDEntries" id=”268”/>
            <uInt32 name="MDUpdateAction" id=”279”> <copy/> </uInt32> 
            <string name="MDEntryType" id=”269”> <copy/> </string> 
            <string name="Symbol" id=”55”> <copy/> </string>
            <string name="SecurityType" id=”167”> <copy/> </string> 
            <decimal name="MDEntryPx" id=”270”> <delta/> </decimal> 
            <decimal name="MDEntrySize" id=”271”> <delta/> </decimal> 
            <uInt32 name="NumberOfOrders" id=”346”> <delta/> </uInt32> 
            <string name="QuoteCondition" id=”276”> <copy/> </string> 
            <string name="TradeCondition" id=”277”> <copy/> </string>
        </sequence> 
    </template> 
    </templates> 

Generating C++ classes from FAST XML specification files.
--------------------------------------------------------

mFAST provide two approaches to incorporate FAST template specification into applications. The first (and preferred) approach is using the tool `fast_type_gen` to  generate C++ types for corresponding FAST templates. Using the generated C++ types  make it easier to read/write application code and provides stronger type safety; in addition, it eliminates the need to parse XML specification at runtime and thus reduce the application memory footprint. The `fast_type_gen` tool is located under the `build/bin` directory. At current stage, it only takes FAST XML files as its arguments and generating C++ files without any options like the following.

    $MFAST_BUILD_DIR/bin/fast_type_gen test1.xml test2.xml

For each XML specification file, `fast_type_gen` would generate 3 files (.h, .inl and .cpp) with the base name of those generated files  the same with their corresponding xml file. The C++ namespace of the generated C++ classes are also the same with the base filename. Currently, There is no other way to change the namespace unless you change the filename of the input XML file or directly modify the generated code.

If your input XML specification files have dependencies among them, all input files must be generated at once; i.e. you cannot do something like `fast_type_gen input1.xml` and then `fast_type_gen input2.xml` if input2.xml is dependent on input1.xml. An input FAST specification file A is only dependent on another file B if and only if B contains a static templateRef where the name of templateRef is only defined in A. 
In addition, `fast_type_gen` cannot handle circular dependency between its input files.

To encoding/decoding FAST messages correctly, each FAST template must have a unique template identifier. The specification does not define how to map template names to template identifier. In our implementation, we choose to use the number specified by `id` attribute in the  template definition as the template identifier for encoding/decoder purposes. In addition, even though the FAST specification does not have any restriction on what the id attributes for fields can be,  `fast_type_gen` can  generate valid C++ code only when the value for id attributes are unsigned 32 bits integers. 

Moreover `fast_type_gen` is not robust enough to report most syntactic or semantic errors from its input files in current stage. It's likely that the code generation process can succeed and yet the generated won't compile or even executed correctly if there are errors in its input files. We may come back to address this issue in the future.

Another approach to use a template specification is to explicitly read the XML file in the application; however, this approach loses type safety and the XML parser would increase the application memory footprint. We will further discuss the API for this approach later.
 

FAST Template to mFAST C++ Type Mapping
==========================================

In order to maximize the space efficiency of  deserialized FAST messages while providing user friendly interfaces,  mFAST employ a very unique FAST template to C++ type mapping.  Unlike OMG IDL/C++ type mapping or Google Protocol Buffer where each message type is represented by a C++ struct/class where the struct/class is responsible for 
resource management and provides members/member functions for accessing fields.
mFAST separates those roles into different C++ classes instead. For the previous mentioned example FAST template  `MDRefreshSample`, 3 C++ classes named `MDRefreshSample`, `MDRefreshSample_cref` and `MDRefreshSample_mref` would be generated by `fast_type_gen`. Based on their functionalities,  we name them as *value holder*, *constant reference* and *mutable reference* classes respectively. 

A value holder class manages the internal resources needed by fields of a message, it does not provide direct interface to read or write its contained fields. The member functions it provides only allows you to construct/copy the value or to create the reference objects.  
Suppose the previous mentioned FAST templates XML is saved as 'example.xml', the generated `MDRefreshSample` would be as follows.

    // example.h, gnerated from `fast_type_gen example.xml`
    namespace example {
    
    class MDRefreshSample_cref
      : public mfast::message_cref {
        // ...
    };
    
    class MDRefreshSample_mref 
      : public MDRefreshSample_cref {
        // ...
    };
    
    class MDRefreshSample
      : private boost::array<mfast::value_storage, 6>
      , public mfast::message_type
    {
      public:
        enum {
          the_id = 10001
        };
        MDRefreshSample(
          mfast::allocator* alloc=mfast::malloc_allocator::instance());
          
        MDRefreshSample(
          const MDRefreshSample_cref& other,
          mfast::allocator* alloc=mfast::malloc_allocator::instance());
          
        MDRefreshSample_cref ref() const;
        MDRefreshSample_cref cref() const;
        MDRefreshSample_mref ref();
        MDRefreshSample_mref mref();
      private:
        MDRefreshSample(const MDRefreshSample&);
        MDRefreshSample& operator = (const MDRefreshSample&);
    };
    
    mfast::templates_description* description();
    
    } // namespace example
    

Constant and mutable reference objects, on the other hand, are used for reading/writing field values, testing the presence of a field, and querying field meta data. 
Each type in FAST specification, whether primitive or not, has its own representative constant/mutable reference classes. 
The following figure is the UML representation for the basic constant reference classes.

![mFAST constant reference class hierarchy](documentation/cref.svg)

At the center of figure is the `field_cref` class, which is the base for all those
const reference classes can be directly used to refer fields. It provides member functions  `present()` for testing the presence of the value it represents, or other functions to query meta data such as name(), id() and field_type().

The classes on the first column are the reference classes for numeric types and 
those on the second column are for string and byte vector types, 
they all provide a `value()` member function to read the field value. 
Classes  in the second column also provide interfaces like the const member functions in `std::string/std::vector<uchar_t>`.
`group_cref` and `sequence_cref`, representing group and sequence types in FAST, provides access to the nested fields or elements respectively. 
`sequenece_element_cref` is used to represents the elements in a sequence; it does not inherit from `field_cref` because it does not corresponds a field in FAST sense. 
`nested_message_cref`, on the other hand, is used to present  templateRef in FAST. 
`message_cref`, corresponding to  *template* in FAST, is the base class for topmost level application types. 
`aggregate_cref` provides an abstraction to represent a collection of fields regardless whether the containing type is a group, a message or a element in a sequence. 
 

Back to the `MDRefreshSample` example, to read the contained fields, we need to use `cref()` to obtain an
`MDRefreshSample_cref` object. The generated `MDRefreshSample_cref` class has a set of `get_xxx()` member functions, where xxx is the field names, used for reading field value. 
Likewise, a set of `set_xxx()` member functions is generated for writing field values in `MDRefreshSample_mref`. 
All the `get_xxx()` and `set_xxx()` member functions return constant and mutable reference objects of their corresponding field types. This is very different from Google protocol buffer where simple value are returned (in `get_xxx()`) or passed as parameter (`in set_xxx()`). 

Here is an example for reading field values in mFAST.

    using namespace mfast;
    void print_sample(const example::MDRefreshSample_cref& ref)
    {       
       cout << ref.get_BeginString().name() << " : " << ref.get_BeginString().c_str() << "\n";
       if (ref.get_ApplVerID().present())
           cout << ref.get_ApplVerID().name() << " : " << ref.get_ApplVerID().c_str() << "\n";
       cout << ref.MsgSeqNum().name() << " : " << ref.MsgSeqNum().value() << "\n";
       
       for (auto entry : ref.MDEntries()) {
           cout << entry.get_MDUpdateAction().name() << " : " << entry.get_MDUpdateAction().c_str();
           // ...
       }      
    }
    
    example::MDRefreshSample message; 
    
    // message value is assigned somewhere else
    
    print_sample(message.cref());
    

All mutable reference classes are derived from the const reference counterpart. In other words,
`int32_cref` is the base class of `int32_mref`, `seqence_cref` is the base class of `sequence_mref`, etc.
However, `field_mref` is not the base class of more specific mutable reference classes for fields such as `int32_mref` or `sequence_mref`; instead, those classes have an explicit type conversion constructor so that it's possible to  `static_cast` from a `field_mref` object to a more specific mutable reference object.
The following code demonstrate how to assign fields values.

    using namespace mfast;
    example::MDRefreshSample message; 
    example::MDRefreshSample_mref ref = message.mref();
    mref.set_ApplVerID().as("A");
    assert(mref.get_ApplVerID().present());
    
    mref.set_SenderCompID.as("B");
    mref.set_MsgSeqNum().as(1);
    auto entries = mref.set_MDEntries();
    entries.resize(4);
    entries[0].set_MDUpdateAction(1);
    // ...
    
    // to make optional field absent after assigning value
    mref.set_ApplVerID().as_absent();
    assert(!mref.get_ApplVerID().present());
    
    // as_absent() has no effect on mandatory fields
    mref.set_MsgSeqNum().as_absent();
    assert(mref.get_ApplVerID().present());
    
    // you can also use clear() to replace as_absent()
    mref.set_ApplVerID().as("C");
    assert(mref.get_ApplVerID().present());
    mref.set_ApplVerID().clear();
    assert(!mref.get_ApplVerID().present());
    
    // However, as_absent() and clear() have different effect on
    // mandatory sequence, string, or byte_vector
    
    assert(mref.set_SenderCompID.size() == 1);
    mref.set_SenderCompID.as_absent(); // no effect
    assert(mref.set_SenderCompID.present());
    assert(mref.set_SenderCompID.size() == 1);
    
    mref.set_SenderCompID.clear(); // becomes zero length string
    assert(mref.set_SenderCompID.present());
    assert(mref.set_SenderCompID.size() == 0);
        

Notice that mFAST reference objects are like C/C++ build-in pointers in that no logic is involved to track the life time of the field/value they refer to; i.e. no reference counting is employed. Once a top level value holder object is destroyed, all the references derived from that value holder object become dangling references. Therefore, it is very important that a reference object cannot be used when its value holder object is destroyed. 
However, it is possible to create a new value holder object from an exiting constant reference object, i.e. object cloning, so that the new value holder have a different life time from which it clones from.

One the other hand, reference objects are small and cheap to copy: a constant reference object contains only 2 pointers internally while a mutable reference object contains 3 pointer. Because no reference counting involved, copying reference object does not impede efficiency by damaging cache performance and creating pipeline bubbles.


Decoding/Encoding FAST Messages
--------------------------  

FAST encoding rule requires both encoder and decoder to agree on a set of template which can be used for the encoded stream. In mFAST, we use a `templates_description` object to package all the meta information on a template set that is
needed by FAST encoder or decoder. For each FAST template XML file, `fast_type_gen` would generated a `templates_description` object named `the_decription` nested inside its own namespace. Before we can use the encoder or decoder, we must initialize it an array of pointer to `templates_description` objects. 
Here an example how to initialize the mFAST decoder.

    const mfast::templates_description* descriptions[] = { 
        example::description()  // generated from fast_type_gen
    };
    
    mfast::fast_decoder decoder;
    
    // In current implmentation, decoder.include() can only be called 
    // exactly once per decoder/encoder
    decoder.include(descriptions);

In certain case, it might not be possible to decide the set of templates to use at compile time.
mFAST also provide an alternative approach to generate the templates_description object at runtime
with a string of FAST XML templates.

    const char* xml_content = "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" ... </templates>";
    mfast::dynamic_templates_description description(xml_content);
    const mfast::templates_description* descriptions[] = { 
        &description
    };
    mfast::fast_decoder decoder;
    decoder.include(descriptions);

After the decoder is properly initialized, we can decode a buffer as follows:

    const char* first = buf;
    const char* last = buf + buf_len;
    
    try {
        whille (first != last) {
            mfast::message_cref msg = decoder.decode(first, last);
            // Caveat 1: the variable *first* would be updated to the start of next unread position after decode().
            // Caveat 2: msg is only valid before next decoder.decode() is called or decoder object is destroyed.
        
            if (msg.id() == MarketData_cref::the_id)
            {
                MarketData_cref sample = static_cast<MarketData_cref>(msg);
                print_sample(sample);
            }
        }
    }
    catch (boost::exeptions& ex) {
        // error handling
    }

Similarly, encoding an application type into a `vector<char>` is as follows:

    
    mfast::fast_encoder encoder;
    encoder.include(descriptions);
    std::vector<char> buffer;
    buffer.reserve(1024);
    
    example::MDRefreshSample message; 
    // ...  write fields into message
    
    encoder.encode(message.cref(), buffer);

Memory Management
------------------------    

To maximize the runtime efficiency, mFAST  has an `mfast::allocator` abstract class for customizing the memory allocation stragey. The constructors of `fast_encoder`, 
`fast_decoder` and all generated types derived from mfast::message_type take a pointer to  `mfast::allocator`
as an optional parameter. If the parameter is not supplied, the system would use a global allocator implemented in  `malloc()`,  `realloc()` and `free()`. If the parameter is supplied, caution must be taken that the supplied 
allocator object must out-live the object, as well as all reference objects derived from it, which the allocator is supplied to.    

Suppose you implemented an allocator class called `memory_pool_alloactor`, the following demonstrate the incorrect and
correct usage of allocators in mFAST.

    
    mfast::message_cref get_message1(const char* buf, size_t sz)
    {
        memory_pool_alloactor alloc;
        mfast::fast_decoder decoder(&alloc);
        decoder.include(descriptions);
        return decoder.decode(buf, buf+sz); // WRONG, the returned message_cref out-lives the decoder and alloc objects which it stems from.
    }
    
    memory_pool_alloactor msg_alloc;
    message_type msg;
    
    void get_message2(const char* buf, size_t sz)
    {
        memory_pool_alloactor alloc;
        mfast::fast_decoder decoder(&alloc);
        decoder.include(descriptions);
        mfast::message_cref ref=decoder.decode(buf, buf+sz); 
        msg = mfast::message_type(ref, &msg_alloc); // OK, copy construct a temporary message_type object 
                                                    // and then move it to msg. mFAST uses boost::move to 
                                                    // simulate rvalue reference in C++03.
    }

Visitor
-----------------

Although it is generally much easier to access application types in structured way using the getters and setters functions as we have introduced, it might not be best way in certain situation such as logging
the received messages in a human readable format.
Using the getters to write a message logger may need to prior knowledge of the application types and the logger functions may differ on the application types. This kind of code tends to be unmanageable as the number of application types grows.

mFAST provides a better way to cope with the above scenario using visitor pattern. 
The visitor pattern allows the separation of an object structure and the operations which operates on it without modifying the structure itself. 
To be more specific in the context mFAST, you write a visitor class which has a set of overloaded `visit()` member functions, each `visit()` member function takes an argument of concrete mFAST field reference types such as `int32_cref`, `ascii_string_cref` and `sequence_cref`, etc. You then create an instance of the visitor class at runtime and invoke the `accept()` member function of `message_cref` or `message_mref` with the parameter of the visitor object.
Next, the `visit()` member functions will be called in the sequence of the fields contained in the message object. 

There are two categories of visitor class in mFAST: *accessor* and *mutator*. An accessor visitor is used to read the fields in a message object; therefore, the `visit()` member functions in accessor visitor class only take constant reference objects as their arguments. Conversely, a mutator visitor is used to modify the fields in a message object so the `visit()` member functions in mutator visitor class only take mutable reference objects as their arguments.

    using namespace mfast;

    // a generic message accessor
    struct message_printer 
    {
	    std::ostream& os_;
	    indenter indent_;
  
        enum {
          visit_absent = 0 // indicating absent fields should not be visited
        };   
        
        message_printer(std::ostream& os)
        : os_(os)
        {
        }

        template <typename PrimitiveTypes>
        void visit(const PrimitiveTypes& ref) const
        { // for int32_cref, uint32_cref, int64_cref, uint64_cref, decimal_cref
          //     ascii_string_cref, unicode_string_cref, byte_vector_cref        
            std::cout << indenter_ << ref.name() << ":" << ref << "\n";
        }
        
	    template <typename CompositeTypeRef>
	    void visit(const CompositeTypeRef& ref, int) const
	    { // for group_cref, sequence_cref, nested_cref
        
	      os_ << indent_ << ref.name() << ":\n";
	      ++indent_;
	      ref.accept_accessor(*this);
	      --indent_;
	    }

	    void visit(const sequence_element_cref&  ref, int index) const
	    {
	      os_ << indent_ <<  "[" << index << "]:\n";
	      ++indent_;
	      ref.accept_accessor(*this);
	      --indent_;
	    }
    };

    std::ostream& operator << (std::ostream& os, const mfast::message_cref& ref)
    {
        message_printer printer(os);
        ref.accept_accessor(printer);
        return os;
    }

A few things have to be noted in the above example.
  - A constant value `visit_absent` must be defined in a visitor class to indicate whether the absent fields should be visitor (0) or not (1).
  - If you are familiar with visitor pattern, you might be puzzled why the visit class does not use inheritance and virtual functions. In fact, we do use inheritance and virtual functions internally. We just wrapped it so that a user  can write generic `visitor()` functions to coalesce several `visit()` member functions into one.
  - The `visit()` function for primitive types always take exactly one argument and those for non-primitive types always take `int` as second argument. This `int` argument is used to differentiation purpose only except for that for sequence_element_cref. In this case, the second argument is the zero based index of the passed-in element in the parent sequence object.
  - In the `visit()` member functions for non-primitive type, the line `ref.accept_accessor(*this);` (or `ref.accept_mutator(*this);` for mutator visitor classes) should be called; otherwise, the fields nested inside the group, sequence or templateRef won't be processed.
 
Evaluation
========================
In this section, we evaluate the memory footprint and runtime performance of applications developed on top of mFAST and QuickFAST.  All the evaluation is performed on a 2009 Macbook Pro with 2.26 GHZ Intel Core 2 Duo CPU. The OS used is Mac OS/X 10.8.4. Applications are compiled with Apple LLVM version 4.2 with libstdc++. Both mFAST and QuickFast use BOOST version 1.54.0. In addition, QuickFAST is dependent on Xeres (version 3.1 used) while mFAST has no such dependency.

Memory Footprint Comparison
-----------------------------------
The core of mFAST is an unique and very compact application type system and all additional functionalities such as  the FAST decoder and encoder  are implemented as visitors and can be linked to applications independently. This makes the mFAST library very modular.  For example, if the FAST encoder is not needed for an application, it wouldn't be  linked into the application and it won't consume the memory footprint of the application executable. On the other hand, QuickFAST does not exhibit such fine grain modularity. The encoder and decoder are in the same module and inseparable without a huge effort to refactor the code. 

To see how small mFAST application is compared to that of QuickFAST. We implemented two test programs for decode a FAST data stream in a file using mFAST. One of them (named fixed_template_test) used  mFAST type generator to produces application types on a sample FAST XML file and then used
the generated type to drive the decoder. The second test program (generic_template_test) directly parsed a FAST specification and
used the parsed information to drive the decoder without the extra code generation step.

On the QuickFAST side, we used the PerformanceTest located in the QuickFAST example directory for comparison.
Like mFAST generic_template_test, the QuickFAST PerformanceTest program parsed a FAST specification on the fly and decoded an input data
stream from a file. However, it did not assemble the decoded messages. Instead, it notified the application via a callback object. Whenever
there's a new data field is processed, the QuickFAST decoder invoked the callback object to provide the name of the data field and the decoded value.

QuickFAST also have a GenericMessageBuilder class which actually build an entire message. In order to provide a functional equivalent program
to our mFAST counterparts (because both fixed_template_test and generic_template_test actually build entire messages), We modified the original PerformanceTest
to use the GenericMessageBuilder (coupled  with a NullMessageConsumer) and named the modified program as GenericBuilderPerformace.

To make fair comparison, both mFAST and QuickFAST libraries are statically linked into their respective applications. However, Xerces and some other BOOST run-time libraries which QuickFAST depended on were still dynamically linked into the executables.
On the other hand,  the BOOST library used by mFAST is header only; there is neither link dependency on BOOST nor on Xerces.

The following  are the sizes of executables compiled  using -O3 flag without debug information.

* mFAST fixed_template_test : 463,296 bytes
* mFAST generic_template_test : 621,076 bytes
* QuickFAST PerformanceTest : 1,317,148 bytes
* QuickFAST GenericBuilderPerformace : 1,450,236 bytes

As you can see, the mFAST executables are about 30% to 40% of the QuickFAST versions; which is quite a reduction, not to say that mFAST versions do not link to other
dynamic libraries as their QuickFAST counterparts.

Better runtime efficiency
----------------------------
To understand mFAST runtime efficiency, I used the above test programs to decode a data stream file with 30000 messages.
The following are the measured performance on my MacBook Pro 2009.

* mFAST fixed_template_test : 142 msec
* mFAST generic_template_test : 133 msec
* QuickFAST PerformanceTest : 148 msec
* QuickFAST GenericBuilderPerformace : 941 msec

Considering that QuickFAST PerformanceTest did not actually assemble the entire message,
it was still slightly slower than mFAST versions with messages being assembled.
In the case where messages were actually assembled, QuickFAST was about 7 times slower
than mFAST.

 
Conclusion
=============

In this article, we introduced the newly designed framework for serializing application data using FAST encoding rule. 
Comparing to QuickFAST, mFAST is slimmer and far more efficient. With the modern design elements, it is also easier to 
develop applications with mFAST rather than QuickFAST. 

However, mFAST at the current stage is not a complete replacement for QuickFAST. First of all, mFAST APIs are entirely different
from QuickFAST. This makes it difficult to migrate existing applications from QuickFAST to mFAST.
Second, QuickFAST has build-in modules for session management while mFAST is purely for encoding/decoding. 
Therefore, if session management is essential, you may want to stick with QuickFAST or contact with support.ociweb.com for support. 
For a foreseeable future, mFAST will remain as a product for serialization purpose only.     

We are very excited about the development of mFAST so far and believe it has  potential to emerge from purely streaming FAST protocol to a more general serialization purpose to transcode among different formats. With the build-in reflective type system and visitor pattern support into its core, it is relatively easy to develop new encoding/decoding formats (such as FIX, JSON, Google Protocol Buffer) non-intrusively.

Currently, mFAST currently supports only FAST specification version 1.1 only. Version 1.2 support is still under development.
