import json,re
import os; R=os.path.dirname(os.path.abspath(__file__))+'/../../'
f=json.load(open('/tmp/report_fork.json'))
json.dump(f,open(R+'progress/report.json','w'))
def col(p): return "lime" if p>=100 else "green" if p>=75 else "yellow" if p>=50 else "darkorange" if p>=25 else "crimson"
def save(name,label,p): json.dump({"schemaVersion":1,"label":label,"message":f"{p:.4f}%","color":col(p)},open(R+f'progress/{name}_progress.json','w'))
cats={c['id']:c['measures'] for c in f['categories']}
for k,m in cats.items(): save(k,k,m.get('matched_functions_percent',0))
M=f['measures']; save('total','Total percentage',M['matched_functions_percent'])
def fmt(m): return '%d / %d (%.1f%%)'%(m.get('matched_functions',0),m['total_functions'],m.get('matched_functions_percent',0))
lines=open(R+'README.md').read().split('\n')
out=[]
for l in lines:
    p=[x.strip() for x in l.split('|')]
    if len(p)==6 and '/' in p[2] and '(' in p[2]:
        key=p[1].strip('`*')
        m=M if key=='Total' else cats.get(key)
        if m:
            fm=fmt(m); cp='%.1f%%'%m.get('matched_code_percent',0)
            if key=='Total': fm='**'+fm+'**'; cp='**'+cp+'**'
            l='| %s | %s | %s | %s | %s'%(p[1],p[2],fm,p[4],cp)
    out.append(l)
open(R+'README.md','w').write('\n'.join(out))
