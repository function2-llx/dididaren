#include "IGTree.h"
#include "G_Tree.h"

G_Tree IGTree::tree;

#include "global.h"

namespace IGTree {
    void init() {
        cerr << "init GTree" << endl;
        // tree = new G_Tree();
        srand(747929791);
        if (!load()) {
            read();
            Additional_Memory=2*G.n*log2(G.n);
            tree.build();
            save();
        }
    }
    void read() {
        fprintf(stderr, "begin read\n");
        FILE *in=NULL;
        in=fopen(Edge_File,"r");
        // cerr<<"correct1"<<endl;
        fscanf(in,"%d %d\n",&G.n,&G.m);
        // cerr<<G.n<<" "<<G.m<<endl;
        // cerr<<"correct2"<<endl;
        G.init(G.n,G.m);
        for(int i=0;i<G.n;i++)G.id[i]=i;
        // cerr<<"correct3"<<endl;
        int i,j,k,l;
        for(i=0;i<G.m;i++)//读取边
        {
            //int temp;
            fscanf(in,"%d %d %d\n",&j,&k,&l);
            if(RevE==false)G.add_D(j,k,l);//单向边
            else G.add(j,k,l);//双向边
        }
        // cerr<<"correct4"<<endl;
        fclose(in);
        if(Optimization_Euclidean_Cut)
        {
            in=fopen(Node_File,"r");
            // cerr<<"correct1"<<endl;
            // cerr<<"correct2"<<endl;
            // cerr<<"correct3"<<endl;
            double d1,d2;
            for(i=0;i<G.n;i++)//读取边
            {
                //int temp;
                fscanf(in,"%d %lf %lf\n",&j,&d1,&d2);
                coordinate.push_back(coor(d1,d2));
            }
            // cerr<<"correct4"<<endl;
            fclose(in);
            fprintf(stderr, "read over\n");
        }
    }

    void save() {
        fprintf(stderr, "begin save\n");
        auto fout = fopen("./data/GP_Tree.data", "w");
        tree.save(fout);
        fclose(fout);
        fprintf(stderr, "save_over\n");
    }

    bool load() {
        auto fin = fopen("./data/GP_Tree.data", "r");
        if (fin == nullptr)
            return 0;
        tree.load(fin);
        fclose(fin);
        return 1;
    }
};