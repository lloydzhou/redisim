import dayjs from 'dayjs'
import relativeTime from 'dayjs/plugin/relativeTime'
dayjs.extend(relativeTime)
export const showTime = (id) => dayjs(parseInt(String(id).split('-')[0])).fromNow()

export const hashCode = s => s.split('').reduce((a,b)=>{a=((a<<5)-a)+b.charCodeAt(0);return a&a},0)
export const colors = ['#ff0000', '#00FF00', '#0000FF']
export const color = (name) => {
  return colors[hashCode(name) % colors.length]
}

export const svg = (content) => 'data:image/svg+xml;utf8,' + encodeURIComponent(content)
export const svgavatar = (name) => svg(`<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="64px" height="64px" viewBox="0 0 64 64" version="1.1"><circle fill="${color(name)}" width="64" height="64" cx="32" cy="32" r="32"/><text x="50%" y="50%" style="color: #ffffff;line-height: 1;font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Fira Sans', 'Droid Sans', 'Helvetica Neue', sans-serif;" alignment-baseline="middle" text-anchor="middle" font-size="26" font-weight="400" dy=".1em" dominant-baseline="middle" fill="#ffffff">${name.slice(0, 2).toUpperCase()}</text></svg>`)

export const chatsvg = (color='#333333') => svg(`<svg t="1666020045935" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="2092" width="200" height="200"><path d="M512 64c259.2 0 469.333333 200.576 469.333333 448s-210.133333 448-469.333333 448a484.48 484.48 0 0 1-232.725333-58.88l-116.394667 50.645333a42.666667 42.666667 0 0 1-58.517333-49.002666l29.76-125.013334C76.629333 703.402667 42.666667 611.477333 42.666667 512 42.666667 264.576 252.8 64 512 64z m0 64C287.488 128 106.666667 300.586667 106.666667 512c0 79.573333 25.557333 155.434667 72.554666 219.285333l5.525334 7.317334 18.709333 24.192-26.965333 113.237333 105.984-46.08 27.477333 15.018667C370.858667 878.229333 439.978667 896 512 896c224.512 0 405.333333-172.586667 405.333333-384S736.512 128 512 128z m-157.696 341.333333a42.666667 42.666667 0 1 1 0 85.333334 42.666667 42.666667 0 0 1 0-85.333334z m159.018667 0a42.666667 42.666667 0 1 1 0 85.333334 42.666667 42.666667 0 0 1 0-85.333334z m158.997333 0a42.666667 42.666667 0 1 1 0 85.333334 42.666667 42.666667 0 0 1 0-85.333334z" fill="${color}" p-id="2093"></path></svg>`)

export const addresssvg = (color='#333333') => svg(`<svg t="1666020279289" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="2237" width="200" height="200"><path d="M518.72 85.333333a203.050667 203.050667 0 0 1 203.050667 203.050667v96.469333c0 56.298667-22.613333 110.229333-62.805334 149.674667l-42.794666 42.026667a3.562667 3.562667 0 0 0-1.066667 2.538666v6.016c0 0.405333 0.234667 0.768 0.597333 0.938667l295.808 139.413333a103.104 103.104 0 0 1 59.157334 93.269334v10.752A87.850667 87.850667 0 0 1 882.816 917.333333H141.184c-46.442667 0-84.8-36.16-87.722667-83.328l-0.128-4.522666v-10.752c0-37.717333 20.586667-72.362667 54.421334-90.88l4.736-2.389334 296.192-139.562666c0.085333 0.042667 0.128 0.256 0.085333 0.746666l0.128-1.536v-6.016c0-0.853333-0.298667-1.642667 0.213333-1.066666l-1.28-1.493334-42.794666-42.005333a210.112 210.112 0 0 1-62.698667-142.613333l-0.106667-7.061334v-96.469333c0-109.781333 87.253333-199.594667 197.226667-202.965333L505.28 85.333333z m0 64h-12.949333l-4.842667 0.064a139.050667 139.050667 0 0 0-134.698667 138.986667v95.872l0.085334 5.909333a145.770667 145.770667 0 0 0 43.562666 98.709334l43.904 43.157333 3.456 3.84c10.112 12.117333 15.658667 27.392 15.658667 43.221333l-0.042667 7.552-0.256 4.693334a65.024 65.024 0 0 1-37.013333 52.608L140.650667 782.912l-2.986667 1.514667a39.104 39.104 0 0 0-20.330667 34.304v9.749333l0.042667 2.538667A23.850667 23.850667 0 0 0 141.184 853.333333h741.632c13.162667 0 23.850667-10.666667 23.850667-23.850666v-10.752c0-15.146667-8.746667-28.928-22.442667-35.392l-295.808-139.392a65.024 65.024 0 0 1-37.312-58.837334v-6.016c0-18.133333 7.296-35.498667 20.224-48.213333l42.794667-42.026667a145.770667 145.770667 0 0 0 43.648-104v-96.469333c0-76.8-62.250667-139.050667-139.050667-139.050667zM951.466667 533.333333c4.693333 0 8.533333 3.84 8.533333 8.533334v46.933333a8.533333 8.533333 0 0 1-8.533333 8.533333h-89.6a8.533333 8.533333 0 0 1-8.533334-8.533333v-46.933333c0-4.693333 3.84-8.533333 8.533334-8.533334h89.6z m0-128c4.693333 0 8.533333 3.84 8.533333 8.533334v46.933333a8.533333 8.533333 0 0 1-8.533333 8.533333h-153.6a8.533333 8.533333 0 0 1-8.533334-8.533333v-46.933333c0-4.693333 3.84-8.533333 8.533334-8.533334h153.6z m0-128c4.693333 0 8.533333 3.84 8.533333 8.533334v46.933333a8.533333 8.533333 0 0 1-8.533333 8.533333h-153.6a8.533333 8.533333 0 0 1-8.533334-8.533333v-46.933333c0-4.693333 3.84-8.533333 8.533334-8.533334h153.6z" fill="${color}" p-id="2238"></path></svg>`)

export const minesvg = (color='#333') => svg(`<svg t="1666059824352" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="11661" width="200" height="200"><path d="M 1013.62 828.311 c -11.4547 -33.2495 -36.6239 -66.5609 -74.7649 -99.0673 c -33.3733 -28.4045 -76.7617 -55.9111 -125.506 -79.5325 a 821.593 821.593 0 0 0 -160.272 -57.7996 C 754.87 540.226 824.774 434.565 824.774 312.821 C 824.774 140.335 684.455 0 511.954 0 S 199.149 140.335 199.149 312.821 c 0 120.429 68.4184 225.161 168.414 277.419 a 787.57 787.57 0 0 0 -155.164 54.2548 C 152.866 672.636 47.9161 732.448 13.7224 816.856 C -8.14983 870.864 4.62056 924.685 49.7271 968.383 c 14.21 13.7611 46.4378 31.1288 170.643 43.5122 c 77.2415 7.73963 177.3 11.9964 281.723 12.1048 h 3.90077 c 103.402 0 203.073 -4.11749 281.042 -11.6404 c 127.271 -12.2596 162.532 -29.7512 178.012 -41.794 c 49.5491 -38.8839 66.8085 -89.3928 48.5739 -142.254 Z m -752.556 -515.46 c 0 -138.385 112.55 -250.934 250.888 -250.934 S 762.857 174.467 762.857 312.821 S 650.323 563.724 511.954 563.724 s -250.888 -112.565 -250.888 -250.903 Z m 665.748 608.954 c -2.19806 1.7182 -24.9061 17.151 -143.539 28.745 c -75.8484 7.41457 -173.77 11.5321 -275.67 11.563 h -1.54793 c -100.786 0 -197.732 -3.96269 -273.147 -11.176 c -116.497 -11.1296 -138.524 -25.8813 -140.258 -27.1816 c -27.0887 -26.3148 -33.9306 -52.9391 -21.5007 -83.588 c 18.9311 -46.7164 81.6377 -98.9125 167.764 -139.638 C 326.079 659.231 426.91 634.65 508.579 634.65 c 82.4581 0 186.293 26.485 277.76 70.8331 c 89.7798 43.5432 152.873 97.0086 168.724 143.044 c 5.99048 17.2439 9.48879 43.6515 -28.2497 73.2479 Z" fill="${color}" p-id="11662"></path><path d="M 628.652 422.971 A 23.2189 23.2189 0 1 0 595.774 390.078 a 118.68 118.68 0 0 1 -167.625 0 a 23.2189 23.2189 0 1 0 -32.8315 32.8315 a 165.164 165.164 0 0 0 233.288 0 Z" fill="${color}" p-id="11663"></path></svg>`)

