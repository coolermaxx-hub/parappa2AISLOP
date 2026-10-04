import sys,os,json
HERE=os.path.dirname(os.path.abspath(__file__))
sys.argv=['x']
exec(open(os.path.join(HERE,'difffn2.py')).read().split('for f in sys.argv')[0])
res={}
for root,_,fs in os.walk('build/src'):
    for f in fs:
        if not f.endswith('.o'): continue
        b=os.path.join(root,f); e='expected2/'+b
        if not os.path.exists(e): continue
        fb,fe=funcs(b),funcs(e)
        bad=sorted(n for n in fe if n in fb and norm(fb[n])!=norm(fe[n]))
        miss=sorted(n for n in fe if n not in fb)
        res[b[10:]]=(bad,miss)
json.dump(res,open(os.path.join(HERE,'alldiff_out.json'),'w'))
print(sum(len(v[0]) for v in res.values()),'diff funcs;',sum(len(v[1]) for v in res.values()),'missing')
