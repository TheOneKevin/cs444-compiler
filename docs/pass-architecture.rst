.. role:: dxc (emphasis)
   :class: dxc

Pass Architecture
================================================================================

Passes are a fundamental building block of the compiler. They allow the compiler to be modular and extensible. This chapter will cover the pass architecture in the compiler.

Overview
--------------------------------------------------------------------------------

A pass is simply a routine that can depend on the results of other passes. A pass inherits from :dxc:`utils::Pass` which is registered with :dxc:`utils::PassManager`. The pass manager is responsible for managing pass resources and dependencies.

The passes registered with a pass manager forms a directed acyclic graph (DAG). When the pass manager is run, it will execute all enabled passes on the DAG in topological order.

By default, all passes are disabled. Then, when a pass is enabled, the pass manager will enable all the passes that the pass depends on. This is done recursively until all dependencies are enabled.

Frontend passes are registered and enabled by the driver, and its DAG is run once. However, the backend optimization passes are repeatedly run. For each pass in the pipeline, only that pass is enabled and the DAG is run. The pass state is persisted between runs.

Resource Acquisition
--------------------------------------------------------------------------------

A pass can acquire heaps from the pass manager. The pass manager is responsible for deducing the lifetimes of a heap. The lifetime of a pass's heap is the lifetime of the last pass to **directly** depend on it in the DAG.

The pass should acquire any heaps that exceeds the pass's lifetime in ``Pass::Init()``. A temporary heap should just be acquired in ``Pass::Run()``. For example, space for analysis results are always acquired in ``Pass::Init()``.

Writing an IR Pass
--------------------------------------------------------------------------------

To write an IR transformation (or analysis) pass, place the file in in ``src/passes/transform/`` (or ``src/passes/analysis/``). The pass should inherit from :dxc:`utils::Pass` and implement the virtual functions as below:

.. code-block:: cpp

    #include "passes/IRContextPass.h"
    #include "tir/Constant.h"
    #include "utils/PassManager.h"

    using std::string_view;
    using utils::Pass;
    using utils::PassManager;

    class HelloWorldPass final : public Pass {
    public:
       HelloWorldPass(PassManager& PM) noexcept : Pass(PM) {}
       void Run() override {
          tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
          // You can do something with CU here
          if(PM().Diag().Verbose())
            PM().Diag().ReportDebug() << "Hello, World!";
       }
       string_view Name() const override { return "helloworld"; }
       string_view Desc() const override { return "Prints Hello World!"; }

    private:
       void computeDependencies() override {
          ComputeDependency(GetPass<IRContextPass>());
       }
    };
    REGISTER_PASS(HelloWorldPass);

Then, you must expose the registration function in ``include/passes/AllPasses.h``:

.. code-block:: cpp

    DECLARE_PASS(HelloWorldPass);

In the jcc1 driver program, you can register the pass with the PM:

.. code-block:: cpp

    NewHelloWorldPass(OptPM);

Automatically, you can use ``-p helloworld`` to run the pass.
