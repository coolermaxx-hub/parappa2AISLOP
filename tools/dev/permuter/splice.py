"""splice.py full.i candidate.c NAME: replace NAME's definition in full.i with the one from candidate.c"""
import sys,re
full=open(sys.argv[1]).read(); cand=open(sys.argv[2]).read(); name=sys.argv[3]
def find(text):
    # find definition: name followed by ( ... ) then {, at line start-ish
    for m in re.finditer(r'(^|\n)([^\n;{}]*\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*)\{',text):
        start=m.start(2); i=m.end()-1; depth=0
        while True:
            c=text[i]
            if c=='{': depth+=1
            elif c=='}':
                depth-=1
                if depth==0: return start,i+1
            i+=1
    raise SystemExit(f'no def of {name}')
a,b=find(full); c,d=find(cand)
sys.stdout.write(full[:a]+cand[c:d]+full[b:])
