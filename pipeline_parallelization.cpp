// pipeline_parallelization.cpp
// Build:  g++ -O2 -std=c++17 pipeline_parallelization.cpp -o pipeline
// Run:    ./pipeline input.txt output.txt
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;

static void die(const char* m){ perror(m); exit(1); }

int main(int argc, char** argv){
    if (argc < 3){ fprintf(stderr,"Usage: %s <input> <output>\n", argv[0]); return 1; }

    const char* inPath  = argv[1];
    const char* outPath = argv[2];

    int p1[2], p2[2];
    if (pipe(p1)==-1) die("pipe p1");
    if (pipe(p2)==-1) die("pipe p2");

    pid_t c1 = fork(); if (c1==-1) die("fork c1");
    if (c1==0){
        // Child 1: transformer (reads p1, writes p2)
        close(p1[1]); close(p2[0]);
        const size_t B=64*1024; vector<char> buf(B);
        ssize_t n;
        while ((n=read(p1[0], buf.data(), B))>0){
            for (ssize_t i=0;i<n;++i){ unsigned char c=buf[i]; if (c>='a'&&c<='z') buf[i]=(char)(c-32); }
            ssize_t off=0; while (off<n){ ssize_t w=write(p2[1], buf.data()+off, n-off); if (w<=0) die("write p2"); off+=w; }
        }
        close(p1[0]); close(p2[1]); _exit(0);
    }

    pid_t c2 = fork(); if (c2==-1) die("fork c2");
    if (c2==0){
        // Child 2: writer (reads p2, writes file)
        close(p1[0]); close(p1[1]); close(p2[1]);
        int outFd=open(outPath, O_CREAT|O_TRUNC|O_WRONLY, 0644); if (outFd==-1) die("open out");
        const size_t B=64*1024; vector<char> buf(B);
        ssize_t n;
        while ((n=read(p2[0], buf.data(), B))>0){
            ssize_t off=0; while (off<n){ ssize_t w=write(outFd, buf.data()+off, n-off); if (w<=0) die("write out"); off+=w; }
        }
        close(p2[0]); close(outFd); _exit(0);
    }

    // Parent: reader (reads file, writes p1)
    close(p1[0]); close(p2[0]); close(p2[1]);
    int inFd=open(inPath,O_RDONLY); if (inFd==-1) die("open in");
    const size_t B=64*1024; vector<char> buf(B);
    ssize_t n;
    while ((n=read(inFd, buf.data(), B))>0){
        ssize_t off=0; while (off<n){ ssize_t w=write(p1[1], buf.data()+off, n-off); if (w<=0) die("write p1"); off+=w; }
    }
    close(inFd); close(p1[1]);
    int st=0; waitpid(c1,&st,0); waitpid(c2,&st,0);
    return 0;
}
