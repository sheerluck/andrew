

txt = """00:00:00 | Disarmonia Mundi - Celestial Furnace
00:03:52 | Rise To Fall - Redrum
00:07:02 | Midnight Realm - Solaris
00:11:27 | Lunarsea - In Expectance
00:17:00 | Deals Death - Concrete Conflict
00:21:14 | Rain Of Acid - Unchained
00:25:39 | The Bereaved - Zero Of The Day
00:29:30 | Nightrage - Spiral
00:34:06 | Sarea - Shattered
00:37:13 | Kiana - Scars
00:40:44 | Aether - Insomnia
00:44:55 | Lahmia - Nightfall
00:49:32 | Mercenary - The Follower
00:54:07 | I Legion - Signs From Above
00:58:03 | Thousand Leaves - Imperfect Woman
01:02:45 | The Hypothesis - Eye For An Eye
01:07:28 | SuperDeathFlame - Untold Burden
01:10:34 | Parasite Inc. - In The Dark
01:14:19 | Zero Degree - Where Angels Die
01:18:25 | Saint Of Disgrace - Bizarro
01:22:30 | Faithful Darkness - Pure Silence
01:27:14 | Dead End Finland - Zero Hour
01:32:04 | All Will Know - Behind Your Mask
01:35:43 | Sunless Rise - Flywheel
01:40:20 | Hero's Fate - Beyond Horizons
01:44:37 | The Stranded - Only Death Can Save Us Now
01:48:57 | Solerrain - The Eternal Lies
01:52:29 | Draconic - Murder The Distance
01:56:37 | Satariel - For Galaxies To Clash
02:00:09 | Descending - Part Of The Game
02:03:55 | Closer - Open Your Eyes
02:08:06 | Engel - Six Feet Deep
02:11:28 | Jotnar - Perfect Lie
02:15:03 | Odium - Serenity's End
02:19:32 | Rifftera - Open Wounds
02:24:54 | Neverborne - Black Roses
02:28:29 | Soul Sacrifice - Torture My Soul
02:32:40 | Laments Of Silence - Rise Again
02:36:39 | With Heavy Hearts - Solar Deity
02:40:19 | Equilibrium Falls - Threshold
02:43:55 | Deathpoint - Lost Haven
02:48:07 | Frequency - Leave The East Side
02:52:25 | Cypecore - Coma Vigil
02:57:17 | Decadawn - Everblack
03:02:04 | Scarpoint - Only Truth
03:06:21 | Recode The Subliminal - Arising
03:10:22 | Aphyxion - As We Blacken The Sky
03:14:13 | Wings In Motion - Our Bleeding
03:18:58 | Dreamsphere - The Punisher
03:23:30 | Distress Of Ruin - In The Heart Of Chaos
03:28:44 | Divided In Spheres - The Dark Passenger
03:33:04 | Eye Of The Enemy - Weight Of Redemption
03:36:49 | Krystalyan - The Cursed Spirit
03:40:56 | In White - Dignity
03:44:00 | Flamorn - I'm Infected
03:48:16 | Elysian - Sigma
03:52:57 | Interra - Your Salvation
03:58:05 | Silent Line - Starfall
04:02:10 | Deadly Curse - Synthetic Humanization
04:06:44 | The Neologist - The Future Screen
04:11:33 | Tria Mera - Illusions
04:15:43 | Archons - A Second Too Late
04:19:20 | Bloodwork - My Order Of One
04:24:06 | T.A.N.K. - Unleash The Craving
04:28:09 | Flayst - Liberation
04:31:54 | Icon In Me - The Quest
04:36:39 | The Deathisodes - Black Virus
04:42:54 | Disfigured Divinity - The Mandatory Heirs
04:46:16 | Code For Silence - Eye For An Eye
04:50:39 | Timecry - The Revelation For The Beggar
04:53:50 | Mekanism - Death Is The Bottom Line
04:58:16 | Empyreon - Infinity
05:02:07 | Marionette - Revelation 6:8
05:05:58 | Manufacturer's Pride - Mind And Machine
05:09:53 | Enthrone The Unborn - Chalice
05:13:43 | Silence Lies Fear - Abiogenesis
05:18:22 | Foredoomed - Blood Red Sun
05:22:44 | Dark Flood - The Fallibles
05:27:52 | D Creation - Killdream
05:33:23 | Fiend - System Error
05:37:09 | Gyze - Regain
05:41:14 | Chaos Injected - Uprising
05:45:26 | Damnation Plan - The Wakening
05:49:29 | Sons Of Senoka - Artificial Symbiotic Error
05:52:45 | Blood Stain Child - Sirius VI
05:57:09 | Sympuls-E - City Of The Void
06:01:34 | Silent Descent - Vortex
06:06:15 | Orpheus Omega - Unblinking
06:10:28 | Degradead - The Dark Mind
06:13:49 | Bloodred Hourglass - Where The Sinners Crawl
06:17:57 | Blinded Colony - Aaron's Sons
06:22:23 | Sonic Syndicate - Jailbreak
06:26:27 | Raunchy - This Legend Forever
06:30:52 | Raintime - Apeiron
06:35:09 | Persefone - Fall To Rise
06:41:19 | Destinity - Black Sun Rising
06:45:02 | Universum - Blank Infinity
06:50:49 | Fractal Gates - Faceless
06:55:42 | Submission - The End Of Eternity
07:03:00 | ROMAN"""

def to_sec(s: str):
    h, m, s = s.split(":")
    H = int(h)
    M = int(m)
    S = int(s)
    return 60*60*H + 60*M + S



fnmain = 'Modern Melodic Death Metal COMPILATION ｜ Unexysted [2CCDnKaYHsg].mkv'

time = 0
for num, (line2, line) in enumerate(zip(txt.split("\n")[1:], txt.split("\n"))):

    n = f"0{num+1}" if num < 9 else f"{num+1}"

    stim, f = line .split(" | ")
    W, Q    = line2.split(" | ")

    f = f.replace("'","_")

    p1 = to_sec(stim)
    p2 = to_sec(W)
    t0 = p2 - p1
    ta = str(p1)

    print(f"ffmpeg -y -loglevel repeat+info -ss {ta}.0 -t {t0}.0 -i 'file:{fnmain}' -c:v copy -c:a libvorbis -map 0 -dn -ignore_unknown -movflags +faststart 'file:{n}_{f}.webm'")


