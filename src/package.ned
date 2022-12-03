package gobackn;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    int n = default(2);
    submodules:
        nodes[n]: Node;
        coordinator: Coordinator;
    connections:
        for i=0..n-1 {
            coordinator.out++ --> nodes[i].in++;
        }
        nodes[0].out --> nodes[1].in++;
        nodes[1].out --> nodes[0].in++;
}

