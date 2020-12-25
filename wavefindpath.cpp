#include <iostream>
#include <vector>
#include "cpp/sc_addr.hpp"
#include "cpp/sc_memory.hpp"
#include "cpp/sc_iterator.hpp"
#include "utils.h"
#include <memory>

ScAddr graph, rrel_arcs, rrel_nodes, graph_nodes, graph_arcs, max_path;
std::unique_ptr<ScMemoryContext> context;

bool set_is_not_empty ( ScAddr& set){
    ScIterator3Ptr check = context->Iterator3(set,ScType::EdgeAccessConstPosPerm,ScType(0));
    if (check->Next()) {
        return true;
    } else {
        return false;
    }
}

void get_edge_vertexes ( ScAddr edge, ScAddr &v1, ScAddr &v2)
{
    v1 = context->GetEdgeSource(edge);
    v2 = context->GetEdgeTarget(edge);
}

bool find_vertex_in_set (ScAddr set, ScAddr vertex)
{
    ScIterator3Ptr location = context->Iterator3(
        set,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    ); // Create iterator to find elements belongs to the set

    while (location->Next()) // Getting every element in set
    {
        ScAddr element = location->Get(2);

        if (vertex == element)
            return true;
    }
    return false;
}

bool find_arc_in_set(const ScAddr& set, ScAddr arc)
{
    ScIterator3Ptr search = context->Iterator3(
        set,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    while (search->Next())
    {
        if (context->GetEdgeTarget(arc) == context->GetEdgeTarget(search->Get(2))
        && context->GetEdgeSource(arc) == context->GetEdgeSource(search->Get(2)))
            return true;
    }
    return false;
}

void add_to_set(const ScAddr& set, const ScAddr& element)
{
    context->CreateEdge(ScType::EdgeAccessConstPosPerm, set, element);
}

void erase_from_set(const ScAddr& set, const ScAddr& element)
{
    ScIterator3Ptr it = context->Iterator3(set,ScType::EdgeAccessConstPosPerm,element);

    if (it->Next())
    {
        ScAddr edge = it->Get(2);
        context->EraseElement(edge);
    }
}

int get_set_size(const ScAddr& set)
{
    int size = 0;
    ScIterator3Ptr it = context->Iterator3(set,ScType::EdgeAccessConstPosPerm,ScType(0));

    while (it->Next())
    {
        size++;
    }
    return size;
}

void clear_set(const ScAddr& set)
{
    ScIterator3Ptr it = context->Iterator3(
        set,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    while (it->Next())
    {
        ScAddr set_element = it->Get(2);
        erase_from_set(set, set_element);
    }
}

void print_graph ()
{
    ScAddr v1, v2, printed_vertex;

    printed_vertex = context->CreateNode(ScType::Const);

    std::cout << "Graph " ;
    printEl(context, graph);
    std::cout<<":" << std::endl;

    ScIterator3Ptr arcs_it = context->Iterator3(graph_arcs,ScType::EdgeAccessConstPosPerm,ScType(0));

    while (arcs_it->Next())
    {
        get_edge_vertexes (arcs_it->Get(2), v1, v2);
        printEl(context, v1);
        std::cout << " => ";
        printEl(context, v2);
        std::cout << std::endl;

        if (!find_vertex_in_set(v1, printed_vertex))
            context->CreateEdge(ScType::EdgeAccessConstPosPerm,printed_vertex, v1);
        if (!find_vertex_in_set(v2, printed_vertex))
            context->CreateEdge(ScType::EdgeAccessConstPosPerm,printed_vertex, v2);
    }

    ScIterator3Ptr nodes_it = context->Iterator3(graph_nodes,ScType::EdgeAccessConstPosPerm,ScType(0));

    while (nodes_it->Next())
    {
        if (find_vertex_in_set(nodes_it->Get(2), printed_vertex))
        {
            printEl(context, nodes_it->Get(2));
            std::cout << std::endl;
        }
    }
    std::cout<<std::endl;
}

void print_max_cut ()
{
    ScAddr v1, v2, printed_vertex;

    printed_vertex = context->CreateNode(ScType::Const);

    std::cout << "Graph max cut:" << std::endl;

    ScIterator3Ptr arcs_it = context->Iterator3(
        graph_arcs,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    while (arcs_it->Next())
    {
        if(!find_arc_in_set(max_path,arcs_it->Get(2)))
        {
            get_edge_vertexes(arcs_it->Get(2), v1, v2);
            printEl(context, v1);
            std::cout << " => ";
            printEl(context, v2);
            std::cout << std::endl;

            if (!find_vertex_in_set(v1, printed_vertex))
                context->CreateEdge(ScType::EdgeAccessConstPosPerm,printed_vertex, v1);
            if (!find_vertex_in_set(v2, printed_vertex))
                context->CreateEdge(ScType::EdgeAccessConstPosPerm,printed_vertex, v2);
        }
    }

    ScIterator3Ptr nodes_it = context->Iterator3(
            graph_nodes,
            ScType::EdgeAccessConstPosPerm,
            ScType(0)
        );

    while (nodes_it->Next())
    {
        if (find_vertex_in_set(printed_vertex, nodes_it->Get(2)))
        {
            printEl(context, nodes_it->Get(2));
            std::cout << std::endl;
        }
    }
    std::cout<<std::endl;
}

void copySet(const ScAddr& destination, const ScAddr& source)
{
    clear_set(destination);

    ScIterator3Ptr it = context->Iterator3(
        source,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    while (it->Next())
    {
        ScAddr source_element = it->Get(2);
        add_to_set(destination, source_element);
    }
}

void depth_search(const ScAddr& edge, const ScAddr& visited_nodes, const ScAddr& path)
{
    ScIterator3Ptr it = context->Iterator3(
            ScType::Node,
            edge,
            ScType::Node);

    ScAddr node;

    if(it->Next())
        node = it->Get(2);

    if (find_vertex_in_set(visited_nodes, node))
        return;

    add_to_set(path, edge);

    int path_length = get_set_size(path);
    int max_path_length = get_set_size(max_path);

    if (path_length > max_path_length)
    {
        //delete all old data in max_path and copy data from path to it
        copySet(max_path, path);
    }

    ScIterator5Ptr incident = context->Iterator5(
            node,
            ScType(0),
            ScType(0),
            ScType::EdgeAccessConstPosPerm,
            graph_arcs);

    while (incident->Next())
    {
        ScAddr edge_to_next_node = incident->Get(1);
        depth_search(edge_to_next_node, visited_nodes, path);
    }

    if(edge.IsValid() && find_arc_in_set(path, edge))
        erase_from_set(path, edge);
}

void find_max_path()
{
    ScIterator3Ptr nodes_it = context->Iterator3(
        graph_nodes,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    while (nodes_it->Next())
    {
        ScAddr node = nodes_it->Get(2);

        ScIterator5Ptr incident = context->Iterator5(
                node,
                ScType(0),
                ScType(0),
                ScType::EdgeAccessConstPosPerm,
                graph_arcs
        );

        while (incident->Next())
        {
            ScAddr edge = incident->Get(1);

            ScAddr visited_nodes = context->CreateNode(ScType::Node);
            ScAddr path = context->CreateNode(ScType::Node);

            add_to_set(visited_nodes, node);
            depth_search(edge, visited_nodes, path);
        }
    }
}


void run_test(const std::string& graph_name)
{
    max_path = context->CreateNode(ScType::Node);

    graph = context->HelperResolveSystemIdtf(graph_name);

    rrel_arcs = context->HelperResolveSystemIdtf("rrel_arcs");
    rrel_nodes = context->HelperResolveSystemIdtf("rrel_nodes");

    ScIterator5Ptr arcs_it = context->Iterator5(
        graph,
        ScType::EdgeAccessConstPosPerm,
        ScType(0),
        ScType::EdgeAccessConstPosPerm,
        rrel_arcs
    );

    if (arcs_it->Next())
    {
        graph_arcs = arcs_it->Get(2);
    }

    ScIterator5Ptr nodes_it = context->Iterator5(
        graph,
        ScType::EdgeAccessConstPosPerm,
        ScType(0),
        ScType::EdgeAccessConstPosPerm,
        rrel_nodes
    );

     if (nodes_it->Next())
    {
        graph_nodes = nodes_it->Get(2);
    }

     print_graph();
     find_max_path();
     if(set_is_not_empty(max_path)){
         std::cout<<"FULL";
     }
     else std::cout<<"EMPTY";

    // print_max_cut();
}

int main()
{
    sc_memory_params params;

    sc_memory_params_clear(&params);
    params.repo_path = "/home/yahor/ostis-example-app/ostis-web-platform/kb.bin";
    params.config_file = "/home/yahor/ostis-example-app/ostis-web-platform/config/sc-web.ini";
    params.ext_path = "/home/yahor/ostis-example-app/ostis-web-platform/sc-machine/bin/extensions";
    params.clear = SC_FALSE;

    ScMemory mem;
    mem.Initialize(params);

    context = std::unique_ptr<ScMemoryContext>(
            new ScMemoryContext(sc_access_lvl_make_max,"example")
    );

    run_test("G0");
    run_test("G1");
    run_test("G2");
    run_test("G3");
    run_test("G4");

    std::cout << "The end" << std::endl;

    mem.Shutdown(true);

    return 0;
}
